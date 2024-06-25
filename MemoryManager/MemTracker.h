#ifndef _MEMTRACKER_H_
#define _MEMTRACKER_H_

#ifndef _MEMORYMANGER_INTERNAL_
#error This is not a public interface, please do not include this file outside of MemoryManager.dll
#endif

#include "DList.h"
#include "FastAllocator.h"

#define DETAILED_LEAK_LOGS 0
//#define LARGE_ALLOCATION_DEBUG_STATS
PUSH_MEMORY_MACROS
#undef new
#undef delete

#pragma warning(push)
#pragma warning(disable: 4351) // warning C4351: new behavior: elements of array will be default initialized

#ifndef ALLOCTYPE_DEFINED
enum AllocType
{
	AllocType_Unknown,
	AllocType_New,
	AllocType_NewAligned,
	AllocType_VectorNew,
	AllocType_VectorNewAligned,
	AllocType_Malloc,
	AllocType_Calloc,
	AllocType_Realloc,
	AllocType_Unvalidated,
};

enum DeallocType
{
	DeallocType_Unknown,
	DeallocType_Realloc,
	DeallocType_Delete,
	DeallocType_DeleteAligned,
	DeallocType_VectorDelete,
	DeallocType_VectorDeleteAligned,
	DeallocType_Free,
	DeallocType_Unvalidated,
};
#define ALLOCTYPE_DEFINED
#endif

class AllocationUnit: public DLNode<AllocationUnit>
{
public:
	AllocationUnit() : 
		ActualAddress(nullptr), 
		ActualSize(0), 
		ReportedAddress(nullptr), 
		ReportedSize(0),
		SourceFile(),
		SourceFunction(),
		SourceLine(-1),
		AllocationType(AllocType_Unknown),
		AllocationNumber((unsigned int)-1),
		AllocationTag(),
		AllocationAlignment(0),
		BreakOnFree(false),
		BreakOnRealloc(false)
#if DETAILED_LEAK_LOGS
		,stack()
#endif
	{

	};

	static void*	operator new(size_t size);
	static void		operator delete(void* memory);

	void*			ActualAddress;
	size_t			ActualSize;
	void*			ReportedAddress;
	size_t			ReportedSize;

	char			SourceFile[128];
	char			SourceFunction[128];
	int				SourceLine;

	AllocType		AllocationType;
	unsigned int	AllocationNumber;
	char			AllocationTag[32];
	unsigned short	AllocationAlignment;

	bool			BreakOnFree;
	bool			BreakOnRealloc;

#if DETAILED_LEAK_LOGS
	unsigned char	stack[512];
#endif
};

class MemoryTrackerThreadLocalInformation
{
public:
	char		CurrentSourceFile[128];
	char		CurrentSourceFunction[128];
	int			CurrentSourceLine;

	char		AllocationTagStack[32][32]; // stack depth = 32 items deep, 32 chars each
	int			CurrentAllocationTag;
};

class MemoryTracker
{
protected:
	enum 
	{
		// how many sentinels on either side of the memory (adjusted for better alignment of user data, prefix can be longer for aligned allocations)
		SENTINEL_SIZE    = (32 - sizeof(AllocationUnit*)) / 4,
		PREFIX_PATTERN   = 0xbaadf00d, // used to fill the bytes preceding allocations
		POSTFIX_PATTERN  = 0xdeadc0de, // used to fill the bytes following allocations
		UNUSED_PATTERN   = 0xfeedface, // used to fill freshly allocated memory
		RELEASED_PATTERN = 0xdeadbeef, // used to fill freshly deallocated memory
	};

	// pointer size + prefix sentinel size
	inline static constexpr size_t unaligned_reported_offset = sizeof(AllocationUnit*) + (SENTINEL_SIZE * sizeof(int32));

	static long			CurrentAllocationCount;

	static inline size_t CalculateAlignedReportedOffset(size_t align)
	{
		return (unaligned_reported_offset + (align - 1_sz)) & (-intptr_t(align));
	}

	static inline void* CalculateActualAddress(void* reported_address, size_t align)
	{
		// memory start - sentinel size - pointer size
		if (align == 0)
			return (char*)reported_address - unaligned_reported_offset;

		return (char*)reported_address - CalculateAlignedReportedOffset(align);
	};
	
	static inline void* CalculateReportedAddress(void* actual_address, size_t align)
	{
		// actual + pointer size + sentinel size 
		if (align == 0)
			return (char*)actual_address + unaligned_reported_offset;

		// aligned addresses have an extended prefix sentinel
		return (char*)actual_address + CalculateAlignedReportedOffset(align);
	};

	static inline void* CalculatePrefixSentinelStartAddress(AllocationUnit* unit)
	{
		// actual + pointer size
		return (char*)unit->ActualAddress + sizeof(AllocationUnit*);
	};

	static inline void* CalculatePostfixSentinelStartAddress(AllocationUnit* unit)
	{
		// memory start + memory size
		return (char*)unit->ReportedAddress + unit->ReportedSize;
	};

	static inline void ClearThreadLocalInformation()
	{
		SetThreadLocalInformation(nullptr, nullptr, -1);
	};

	static size_t		GetMemoryTrackingCost(size_t align);
	static AllocationUnit*	FindAllocationUnit(void* memory, size_t align);
	static void				WipeMemoryWithPattern(AllocationUnit* unit, int32 pattern, size_t bias = 0);
	static bool				ValidateAllocationUnit(AllocationUnit* unit);
public:
	~MemoryTracker();
	static MemoryTrackerThreadLocalInformation* GetThreadLocalInformation();
	static void		SetThreadLocalInformation(const char* source_file, const char* source_function, const int source_line);
	static void		PushAllocationTag(const char* tag);
	static void		PopAllocationTag();
	DECLSPEC_RESTRICT static void* Allocate(const size_t size, const size_t align, const AllocType type = AllocType_Unknown, const char* source_file = "(Unknown File)", const char* source_function = "(Unknown Function)", const int source_line = -1);
	DECLSPEC_RESTRICT static void* Reallocate(void* memory, const size_t size, const char* source_file = "(Unknown File)", const char* source_function = "(Unknown Function)", const int source_line = -1);
	static void		Free(void* memory, const size_t align, const DeallocType type = DeallocType_Unknown, const char* source_file = "(Unknown File)", const char* source_function = "(Unknown Function)", const int source_line = -1);
	static size_t	GetAllocationSize(void* memory);
};

#pragma warning(pop)
POP_MEMORY_MACROS

#endif
