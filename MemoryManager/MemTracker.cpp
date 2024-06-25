#include "general.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "winmm.lib")
#include "MemTracker.h"
#include "FastAllocator.h"
#include "FastCriticalSection.h"
#include "MemoryManager.h"

// NOTE: removed extremely expensive includes just for these function declarations, <shlobj.h> and <shlwapi.h>
EXTERN_C DECLSPEC_IMPORT int STDAPICALLTYPE SHCreateDirectoryExA(_In_opt_ HWND hwnd, _In_ LPCSTR pszPath, _In_opt_ const SECURITY_ATTRIBUTES* psa);
EXTERN_C DECLSPEC_IMPORT BOOL STDAPICALLTYPE PathFileExistsA(_In_ LPCSTR pszPath);

//#define ALWAYS_WIPE_MEMORY
#define LEAK_LOG_DIR "debug\\memory"

#pragma comment(lib, "shlwapi.lib")

#pragma warning(push)
#pragma warning(disable: 4073) // warning C4073: initializers put in library initialization area
#pragma init_seg(lib) // leet hax to insure our threads and friends are initialized before everybody else
#pragma warning(pop)
PUSH_MEMORY_MACROS
#undef new
#undef delete

static FastCriticalSection					MemoryTrackerCS;
static FastFixedAllocator					TagAllocator(sizeof(AllocationUnit));
static DList<AllocationUnit>				Tags;
// ReSharper disable once CppDeclaratorNeverUsed
static MemoryTracker						MemoryTrackerInstance;
#ifdef LARGE_ALLOCATION_DEBUG_STATS
DWORD StartTime = timeGetTime();
#endif

void* AllocationUnit::operator new(size_t size)
{
	assert(size == sizeof(AllocationUnit));
	return TagAllocator.Allocate();
};

void AllocationUnit::operator delete(void* memory)
{
	TagAllocator.Free(memory);
};


MemoryTracker::~MemoryTracker()
{
	char path_to_exe[256];
	GetModuleFileNameA(nullptr, path_to_exe, sizeof(path_to_exe));
	strrchr(path_to_exe,'\\')[0] = 0;
	SetCurrentDirectoryA(path_to_exe);
	strncat_s(path_to_exe,"\\", _TRUNCATE);
	strncat_s(path_to_exe,LEAK_LOG_DIR, _TRUNCATE);
	if (!PathFileExistsA(path_to_exe))
	{
		SHCreateDirectoryExA(nullptr,path_to_exe, nullptr);
	}
	FILE* leakLogFile = nullptr;
	errno_t error = fopen_s(&leakLogFile, LEAK_LOG_DIR "\\leaks.txt", "wb");
	if (error)
		OutputDebugStringA("Failed to write to " LEAK_LOG_DIR "\\leaks.txt");
	else
	{
		fprintf(leakLogFile, "address\tsize\tfile\tfunction\tline\tallocation type\r\n");
		int LeakCount = 0;
		for (AllocationUnit* tag = Tags.PopHead(); tag; tag = Tags.PopHead() )
		{
			const char* allocationTypeName = nullptr;
			switch (tag->AllocationType)
			{
			case AllocType_Unknown: allocationTypeName = "unknown"; break;
			case AllocType_New: allocationTypeName = "new"; break;
			case AllocType_NewAligned: allocationTypeName = "new (aligned)"; break;
			case AllocType_VectorNew: allocationTypeName = "new[]"; break;
			case AllocType_VectorNewAligned: allocationTypeName = "new[] (aligned)"; break;
			case AllocType_Malloc: allocationTypeName = "malloc"; break;
			case AllocType_Calloc: allocationTypeName = "calloc"; break;
			case AllocType_Realloc: allocationTypeName = "realloc"; break;
			case AllocType_Unvalidated: allocationTypeName = "unvalidated"; break;
			default: __assume(false);
			}
			LeakCount++;
			fprintf(leakLogFile, "0x%8p\t0x%04IX (%Iu)\t%.128s\t%.128s\t%d\t%s\r\n", tag->ReportedAddress, tag->ReportedSize, tag->ReportedSize, tag->SourceFile, tag->SourceFunction, tag->SourceLine, allocationTypeName);

	#if DETAILED_LEAK_LOGS
			char detailedLeakLogFilePath[MAX_PATH];
			snprintf(detailedLeakLogFilePath, sizeof(detailedLeakLogFilePath), LEAK_LOG_DIR "\\leak-stack-%08p.txt", tag->ReportedAddress);
			FILE* detailedLeakLog = fopen(detailedLeakLogFilePath, "wb");
			fwrite(tag->stack, 1, sizeof(tag->stack), detailedLeakLog);
			fclose(detailedLeakLog);

			snprintf(detailedLeakLogFilePath, sizeof(detailedLeakLogFilePath), LEAK_LOG_DIR "\\leak-data-%08p.txt", tag->ReportedAddress);
			detailedLeakLog = fopen(detailedLeakLogFilePath, "wb");
			fwrite(tag->ReportedAddress, 1, tag->ReportedSize, detailedLeakLog);
			fclose(detailedLeakLog);
	#endif

			delete tag;
		}
		if (LeakCount)
		{
			char buffer[256];
			snprintf(buffer, sizeof(buffer),"%d Memory Leaks found\n",LeakCount);
			OutputDebugStringA(buffer);
		}
		fclose(leakLogFile);
	}
};


long MemoryTracker::CurrentAllocationCount;

// each memory allocation looks like
// pointer to AllocationUnit instance
// start padding
// actual memory
// end padding

static const char* FilePathStripper(const char *source_file)
{
	const char* ptr = strrchr(source_file, '\\');
	if (ptr) return ptr + 1;
	ptr = strrchr(source_file, '/');
	if (ptr) return ptr + 1;
	return source_file;
}

size_t MemoryTracker::GetMemoryTrackingCost(size_t align)
{
	constexpr size_t tracking_cost = sizeof(AllocationUnit*) + (SENTINEL_SIZE * sizeof(int32) * 2); // pointer size + all sentinels size
	
	if (align == 0)
		return tracking_cost;

	// calculate where the actual user data would end up
	size_t aligned_reported_offset = CalculateAlignedReportedOffset(align);
	// correct for alignment of user data by extending the prefix sentinel
	return tracking_cost + (aligned_reported_offset - unaligned_reported_offset);
};

AllocationUnit* MemoryTracker::FindAllocationUnit(void* memory, size_t align)
{
	void* actual = CalculateActualAddress(memory, align);
	int32* sentinel_start = (int32*)((char*) actual + sizeof(AllocationUnit*));
	if (*sentinel_start != PREFIX_PATTERN) return nullptr; // preliminary validation
	return *(AllocationUnit**) actual;
};

void MemoryTracker::WipeMemoryWithPattern(AllocationUnit* unit, int32 pattern, size_t bias)
{

#ifdef ALWAYS_WIPE_MEMORY

	// fill the bulk of the allocation
	int32* iptr = (int32*)((char*)unit->ReportedAddress + bias);
	size_t length = unit->ReportedSize - bias;
	for (size_t i = 0; i < (length / 4); ++i)
	{
		*++iptr = pattern;
	};

	// fill the remainder	
	char* cptr = (char*)iptr;
	for (size_t i = 0, shiftcount = 0; i < (length & 0x3); ++i, shiftcount += 8)
	{
		*++cptr = char(pattern & (0xFF << shiftcount) >> shiftcount);
	}
#else
	pattern;
	bias;
#endif

	int32 * __restrict pre = (int32*)CalculatePrefixSentinelStartAddress(unit);
	int32 * __restrict post = (int32*)CalculatePostfixSentinelStartAddress(unit);
	for (size_t i = 0; i < SENTINEL_SIZE; ++i, ++pre, ++post)
	{
		*pre = PREFIX_PATTERN;
		*post = POSTFIX_PATTERN;
	}
	// prefix can be longer for aligned allocations
	while (pre < unit->ReportedAddress)
	{
		*pre++ = PREFIX_PATTERN;
	}
};


bool MemoryTracker::ValidateAllocationUnit(AllocationUnit* unit)
{
	// make sure the sentinels are untouched
	int32 * __restrict pre = (int32*)CalculatePrefixSentinelStartAddress(unit);
	int32 * __restrict post = (int32*)CalculatePostfixSentinelStartAddress(unit);
	bool error_flag = false;
	for (size_t i = 0; i < SENTINEL_SIZE; ++i, ++pre, ++post)
	{
		if (*pre != PREFIX_PATTERN)
		{
			// log this event maybe?
			error_flag = true;
		}

		if (*post != POSTFIX_PATTERN) 
		{
			// this one too?
			error_flag = true;
		}
		
		// If you hit this assert, the sentinel before the allocation has been damaged 
		assert(*pre == PREFIX_PATTERN);

		// If you hit this assert, the sentinel after the allocation has been damaged
		assert(*post == POSTFIX_PATTERN);
	}

	// prefix can be longer for aligned allocations
	while (pre < unit->ReportedAddress)
	{
		if (*pre != PREFIX_PATTERN)
		{
			// log this event maybe?
			error_flag = true;
		}

		// If you hit this assert, the sentinel before the allocation has been damaged 
		assert(*pre == PREFIX_PATTERN);

		pre++;
	}
	
	return !error_flag;
};

extern DWORD TLSIndex; // HACK: Defined in dllmain_debug
MemoryTrackerThreadLocalInformation* MemoryTracker::GetThreadLocalInformation()
{
	void* tls_mem = TlsGetValue(TLSIndex);
	return (MemoryTrackerThreadLocalInformation*)tls_mem;
};

void MemoryTracker::SetThreadLocalInformation(const char* source_file, const char* source_function, const int source_line)
{
	MemoryTrackerThreadLocalInformation* info = GetThreadLocalInformation();
	strncpy_s(info->CurrentSourceFile, source_file ? FilePathStripper(source_file) : "(Unknown File)", _TRUNCATE);
	strncpy_s(info->CurrentSourceFunction, source_function ? source_function : "(Unknown Function)", _TRUNCATE);
	info->CurrentSourceLine = source_line;
};

void MemoryTracker::PushAllocationTag(const char* tag)
{
	MemoryTrackerThreadLocalInformation* info = GetThreadLocalInformation();
	strcpy_s(info->AllocationTagStack[++info->CurrentAllocationTag], sizeof(info->AllocationTagStack[0]), tag);
};

void MemoryTracker::PopAllocationTag()
{
	MemoryTrackerThreadLocalInformation* info = GetThreadLocalInformation();
	--info->CurrentAllocationTag;
};

DECLSPEC_RESTRICT void* MemoryTracker::Allocate(const size_t size, const size_t align, const AllocType type, const char* source_file, const char* source_function, const int source_line)
{
	// If you hit this assert, then this allocation call was made from a source that isn't setup to use our
	// memory tracking system. Use the callstack to locate the source and include our memory tracker header.
	assert(type != AllocType_Unknown);
	
	AllocationUnit* tag = new AllocationUnit();

	// If you've hit this assert, then you've run out of memory for the allocation tag, which is really really bad.
	assert(tag != nullptr);

	tag->ActualSize = GetMemoryTrackingCost(align) + size;
	if (align == 0)
		tag->ActualAddress = FastAllocatorGeneral::Get_Allocator()->Allocate(tag->ActualSize);
	else
		tag->ActualAddress = FastAllocatorGeneral::Get_Allocator()->Allocate(tag->ActualSize, align);

	// If you've hit this assert, then you've run out of memory
	assert(tag->ActualAddress != nullptr);

	tag->ReportedSize = size;
	tag->ReportedAddress = CalculateReportedAddress(tag->ActualAddress, align);
	tag->AllocationType = type;
	tag->AllocationAlignment = unsigned short(align);
	
	if (source_file) strncpy_s(tag->SourceFile, FilePathStripper(source_file), _TRUNCATE);
	if (source_function) strncpy_s(tag->SourceFunction, source_function, _TRUNCATE);
	tag->SourceLine	= source_line;

	{
	// Since we are getting the tracking info here anyways we could optimize this function to just use the info from here
	MemoryTrackerThreadLocalInformation* info = GetThreadLocalInformation();
	strcpy_s(tag->AllocationTag, sizeof(tag->AllocationTag), info->AllocationTagStack[info->CurrentAllocationTag]);
	}

	tag->AllocationType = type;
	tag->AllocationNumber = _InterlockedIncrement((long*)&CurrentAllocationCount);

	tag->BreakOnFree = false;
	tag->BreakOnRealloc = false;

#if DETAILED_LEAK_LOGS
	// Quick hack to avoid going out of stack bounds. TODOx: Find a better solution.
	memcpy(tag->stack, _AddressOfReturnAddress(), min(0x1000 - (uint)_AddressOfReturnAddress() & (0x1000-1), sizeof(tag->stack)));
#endif

	// Store the allocation unit tag address within the allocation memory
	*(AllocationUnit**)tag->ActualAddress = tag;

#ifdef LARGE_ALLOCATION_DEBUG_STATS
	if (tag->ReportedSize > (1 << 13)) // big memory
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer), 
			"%.3f\talloc:\t(%dB) <%.3fMiB> %s %s:%d\n", 
			(timeGetTime() - StartTime) / 1000.0f, 
			tag->ReportedSize, 
			FastAllocatorGeneral::Get_Allocator()->GetTotalMemoryUsage() / 1024.f / 1024.0f, 
			tag->SourceFunction, 
			tag->SourceFile, 
			tag->SourceLine);
		OutputDebugStringA(buffer);
	}
#endif

	// We'll wipe the memory here with our "Unused" pattern so we can later estimate how much of that allocated
	// memory was actually "used"
	WipeMemoryWithPattern(tag, UNUSED_PATTERN);

	// And we'll wipe the memory here *again* if it's allocated via calloc, which expects all the memory to be zero-init'd
	if (type == AllocType_Calloc) memset(tag->ReportedAddress, 0x00, tag->ReportedSize);

	// Clearing the tracking information insures that if at some later time somebody calls our memory tracker
	// from an unknown source, we don't think it was the last allocation.
	ClearThreadLocalInformation();

	// Add the memory tag to our "tracked" list.
	{
		FastCriticalSection::Lock lock(MemoryTrackerCS);
		Tags.PushTail(tag);
	}

	return tag->ReportedAddress;
};

DECLSPEC_RESTRICT void* MemoryTracker::Reallocate(void* memory, const size_t size, const char* source_file, const char* source_function, const int source_line)
{
	// standard defines realloc as returning malloc(size) if memblock is NULL
	if (!memory) return MemoryTracker::Allocate(size, 0, AllocType_Realloc, source_file, source_function, source_line);

	// standard defines realloc as calling free(memory) and returning NULL if size is NULL
	if (!size)
	{
		MemoryTracker::Free(memory, 0, DeallocType_Realloc, source_file, source_function, source_line);
		return nullptr;
	}

	AllocationUnit* tag = FindAllocationUnit(memory, 0);

	// If you hit this assert, you tried to reallocate memory that was either not allocated with this allocator 
	// or so badly damaged that it failed the preliminary test
	assert(tag != nullptr);

	// We've warned the user about the tag being NULL, let's not actually try to do anything further with the memory
	if (tag == nullptr) return nullptr;

	// If you hit this assert, the memory that is about to be reallocated is damaged, but you should have seen
	// an earlier assert in ValidateAllocationUnit
	assert(ValidateAllocationUnit(tag));

	// If you hit this assert, you were trying to reallocate RAM that wasn't allocated in a way that's compatible
	// with reallocation. Simply put, you can't use new/new[] with realloc
	assert((tag->AllocationType != AllocType_New)        && (tag->AllocationType != AllocType_VectorNew) &&
	       (tag->AllocationType != AllocType_NewAligned) && (tag->AllocationType != AllocType_VectorNewAligned));

	// Allocate the new memory and copy over the existing contents
	void* new_memory = MemoryTracker::Allocate(size, 0, AllocType_Realloc, source_file, source_function, source_line);
	memcpy(new_memory, memory, size);

	// Copy over BreakOn* info
	AllocationUnit* new_tag = FindAllocationUnit(new_memory, 0);
	new_tag->BreakOnFree = tag->BreakOnFree;
	new_tag->BreakOnRealloc = tag->BreakOnRealloc;

	// Free the old memory
	MemoryTracker::Free(memory, 0, DeallocType_Realloc, source_file, source_function, source_line);

	return new_memory;
};

void MemoryTracker::Free(void* memory, const size_t align, const DeallocType type, const char* /*source_file*/, const char* /*source_function*/, const int /*source_line*/)
{
	if (!memory) return; // standard defines free(NULL) as basically a no-op, so do so here.
	AllocationUnit* tag = FindAllocationUnit(memory, align);

	// If you hit this assert, you tried to deallocate memory that was either not allocated with this allocator 
	// or so badly damaged that it failed the preliminary test
	assert(tag != nullptr);

	// We've warned the user about the tag being NULL, let's not actually try to do anything further with the memory
	if (tag == nullptr) return;

	// If you hit this assert, the memory that is about to be deallocated is damaged, but you should have seen
	// an earlier assert in ValidateAllocationUnit
	assert(ValidateAllocationUnit(tag));

	// If you hit this assert, then this deallocation call was made from a source that isn't setup to use our
	// memory tracking system. Use the callstack to locate the source and include our memory tracker header.
	assert(type != DeallocType_Unknown);

	// If you hit this assert, you were trying to deallocate RAM that wasn't allocated in a way that's compatible
	// with the deallocation method requested. Simply put, your allocation/deallocation calls are mismatched.
	assert (
		(tag->AllocationType == AllocType_New              && type == DeallocType_Delete) ||
		(tag->AllocationType == AllocType_NewAligned       && type == DeallocType_DeleteAligned) ||
		(tag->AllocationType == AllocType_VectorNew	       && type == DeallocType_VectorDelete) ||
		(tag->AllocationType == AllocType_VectorNewAligned && type == DeallocType_VectorDeleteAligned) ||
		(tag->AllocationType == AllocType_Malloc           && type == DeallocType_Free) ||
		(tag->AllocationType == AllocType_Calloc           && type == DeallocType_Free) ||
		(tag->AllocationType == AllocType_Realloc          && type == DeallocType_Free) ||
		(tag->AllocationType == AllocType_Malloc           && type == DeallocType_Realloc) ||
		(tag->AllocationType == AllocType_Calloc           && type == DeallocType_Realloc) ||
		(tag->AllocationType == AllocType_Realloc          && type == DeallocType_Realloc) ||
		(tag->AllocationType == AllocType_Unvalidated) || 
		(type == DeallocType_Unvalidated)
	);

	// If you hit this assert, you requested that we break when this piece of memory came through to be deallocated/reallocated
	assert(!(tag->BreakOnFree && type == DeallocType_Free) || !(tag->BreakOnRealloc && type == DeallocType_Realloc));
	
#ifdef LARGE_ALLOCATION_DEBUG_STATS
	if (tag->ReportedSize > (1 << 13)) // big memory
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer), "%.3f\tfree:\t(%d bytes) <%.3fMiB> %s %s:%d\n", 
			(timeGetTime() - StartTime) / 1000.0f, 
			tag->ReportedSize, 
			FastAllocatorGeneral::Get_Allocator()->GetTotalMemoryUsage() / 1024.f / 1024.0f, 
			tag->SourceFunction, 
			tag->SourceFile, 
			tag->SourceLine);
		OutputDebugStringA(buffer);
	}
#endif
	
	// We'll wipe the memory here with our "Released" pattern even though it likely won't do much good.
	WipeMemoryWithPattern(tag, RELEASED_PATTERN);

	// Nuke the allocation unit tag address
	*(AllocationUnit**)tag->ActualAddress = nullptr;

	// Free the actual memory
	if (align == 0)
		FastAllocatorGeneral::Get_Allocator()->Free(tag->ActualAddress);
	else
		FastAllocatorGeneral::Get_Allocator()->Free(tag->ActualAddress, align);

	// Clearing the tracking information insures that if at some later time somebody calls our memory tracker
	// from an unknown source, we don't think it was the last allocation.
	ClearThreadLocalInformation();

	// Let's also stop tracking the now free'd memory and free our tracking tag.
	{
		FastCriticalSection::Lock lock(MemoryTrackerCS);
		tag->Remove();
		delete tag;
	}
};

size_t MemoryTracker::GetAllocationSize(void* memory)
{
	// FIXME: For aligned allocations we need to pass the alignment here, otherwise the asserts will fire and the result will be garbage.
	AllocationUnit* tag = FindAllocationUnit(memory, 0);

	// If you hit this assert, you tried to get the size of memory that was either not allocated with this allocator 
	// or so badly damaged that it failed the preliminary test
	assert(tag != nullptr);

	// We've warned the user about the tag being NULL, let's not actually try to do anything further with the memory
	if (tag == nullptr) return 0;

	// If you hit this assert, the memory that is about to be deallocated is damaged, but you should have seen
	// an earlier assert in ValidateAllocationUnit
	assert(ValidateAllocationUnit(tag));

	return tag->ReportedSize;
};

POP_MEMORY_MACROS
