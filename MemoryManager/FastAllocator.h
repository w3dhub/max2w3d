// Do you know what you are doing?
// No? Then don't touch this file.
#ifndef _FASTALLOCATOR_H_
#define _FASTALLOCATOR_H_

#include "platform.h"
#if defined(_DEBUG) && defined(WIN32) && defined(_M_IX86) // FASTALLOC_STATS only works for x86 windows builds
#define FASTALLOC_STATS
#endif
#include "TSList.h"
#include "MemoryManager.h"
PUSH_MEMORY_MACROS
#undef new
#undef delete

class SystemAllocatedClass
{
protected:
	DECLSPEC_RESTRICT static inline void* SystemAllocate(size_t size)
	{
		return malloc(size);
	}
	static inline void SystemFree(void* memory)
	{
		free(memory);
	}
public:
	DECLSPEC_RESTRICT static inline void* operator new(size_t size)
	{
		return SystemAllocate(size);
	}
	static inline void operator delete(void* memory)
	{
		SystemFree(memory);
	}
	DECLSPEC_RESTRICT static inline void* operator new[](size_t size)
	{
		return SystemAllocate(size);
	}
	static inline void operator delete[](void* memory)
	{
		SystemFree(memory);
	}
};

class VirtualAllocatedClass
{
protected:
	DECLSPEC_RESTRICT static inline void* VirtualAllocate(size_t size)
	{
#ifdef MEMORY_64BIT_ALLOCATION_DEBUG
		// Randomly allocate from the top or bottom so that we might more quickly end up with different pointers that have matching lower 32 bits
		// Note that MEM_TOP_DOWN is fairly slow, so we should not have this enabled in release builds.
		return ::VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT | ((rand() & 1) ? MEM_TOP_DOWN : 0), PAGE_READWRITE);
#else
		return ::VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#endif
	}
	static inline void VirtualFree(void* memory)
	{
		::VirtualFree(memory, 0, MEM_RELEASE);
	}
public:
#ifndef DISABLE_CUSTOM_ALLOCATORS
	DECLSPEC_RESTRICT static inline void* operator new(size_t size)
	{
		return VirtualAllocate(size);
	}
	static inline void operator delete(void* memory)
	{
		VirtualFree(memory);
	}
	DECLSPEC_RESTRICT static inline void* operator new[](size_t size)
	{
		return VirtualAllocate(size);
	}
	static inline void operator delete[](void* memory)
	{
		VirtualFree(memory);
	}
#endif
};

class FastFixedAllocator : public SystemAllocatedClass
{
protected:
	struct Link : public TSLNode
	{
	};
	// Virtual allocated means our chunks are always aligned to at least page size (GetSystemInfo -> dwAllocationGranularity, usually 64 KiB).
	// Note however that the TSLNode offset misaligns our actual allocations.
	struct Chunk : public VirtualAllocatedClass
	{
		enum : size_t
		{
			size = 16 * 64 * 1024 - sizeof(TSLNode) // 1 MiB
		};
		TSLNode			Node;
		char			Memory[size];
	};

	// NOTE(Dgh): This is currently a hard requirement for the rest of the code to not misalign the returned memory.
	static_assert(offsetof(FastFixedAllocator::Chunk, Memory) == PLATFORM_MEMORY_ALIGNMENT);

	TSList<Link>	FreeList;
	TSList<Chunk>	Chunks;
	size_t			ElementSize;
#ifdef FASTALLOC_STATS
public:
	unsigned long TotalHeapSize;
	unsigned long TotalAllocatedSize;
	unsigned long TotalAllocationCount;
protected:
#endif
public:
	template<typename T>
	static FastFixedAllocator CreateForType()
	{
		return FastFixedAllocator(sizeof(T));
	}
	FastFixedAllocator(size_t element_size = 1);
	~FastFixedAllocator();
	void Init(size_t element_size);
	void Reset();
	virtual void Grow();

	inline size_t GetSize() const { return ElementSize; }

	DECLSPEC_RESTRICT void* Allocate();
	void Free(void* memory);

	friend class FastAllocatorGeneral;
};

inline FastFixedAllocator::FastFixedAllocator(size_t element_size): 
FreeList(),
Chunks(), 
ElementSize(1)
#ifdef FASTALLOC_STATS
,
TotalHeapSize(0),
TotalAllocatedSize(0),
TotalAllocationCount(0)
#endif
{
	Init(element_size);
}

inline FastFixedAllocator::~FastFixedAllocator() // *not* thread safe
{
	Reset();
}

inline void FastFixedAllocator::Reset() // *not* thread safe
{
#ifndef DISABLE_CUSTOM_ALLOCATORS
	for (Chunk* n = (Chunk*)Chunks.Pop(); n; n = Chunks.Pop())
	{
		delete n;
	}
	FreeList.ResetHead();
	Chunks.ResetHead();

#ifdef FASTALLOC_STATS
	TotalAllocationCount = 0;
	TotalAllocatedSize = 0;
	TotalHeapSize = 0;
#endif
#endif
}

inline void FastFixedAllocator::Grow()
{
#ifndef DISABLE_CUSTOM_ALLOCATORS
	Chunk* n = new Chunk;
	Chunks.Push(n);

#ifdef FASTALLOC_STATS
	_InterlockedExchangeAdd((long *)&TotalHeapSize, sizeof(Chunk));
#endif

	const size_t number_elements = Chunk::size / ElementSize;

	for (size_t i = 0; i < number_elements; ++i)
	{
		Link* link = (Link*)(n->Memory + (i * ElementSize) );
		link->Next = nullptr;
		FreeList.Push(link);
	}
#endif
}

inline void FastFixedAllocator::Init(size_t element_size)
{
	this->ElementSize = element_size < sizeof(Link) ? sizeof(Link) : element_size;
	assert(ElementSize < Chunk::size);
}

DECLSPEC_RESTRICT inline void* FastFixedAllocator::Allocate()
{
#ifdef DISABLE_CUSTOM_ALLOCATORS
	return malloc(ElementSize);
#else
#ifdef FASTALLOC_STATS
	_InterlockedIncrement((long *)&TotalAllocationCount);
	_InterlockedExchangeAdd((long *)&TotalAllocatedSize, (long)ElementSize);
#endif

	void* memory = FreeList.Pop();
	while (!memory)
	{
		Grow();
		memory = FreeList.Pop();
		YieldThread();
	}
	return memory;
#endif
}

inline void FastFixedAllocator::Free(void* memory)
{
#ifdef DISABLE_CUSTOM_ALLOCATORS
	return free(memory);
#else
	if (memory)
	{
#ifdef FASTALLOC_STATS
		_InterlockedDecrement((long*)& TotalAllocationCount);
		_InterlockedExchangeAdd((long*)& TotalAllocatedSize, -(long)ElementSize);
#endif

		FreeList.Push((Link*)memory);
	}
#endif
}

// Variant of FastFixedAllocator that aligns all elements to a specified alignment.
// Use offset if you want to align a specific part of the element that is not at the beginning of the struct.
// Don't use ridiculous alignments, offsets and element sizes or you risk wasting a lot of space.
// If you need very large amounts of space with large alignments, just use VirtualAlloc directly.
class FastFixedAlignedAllocator : public FastFixedAllocator
{
	size_t Alignment;
	size_t Offset;
public:

	template<typename T>
	static FastFixedAlignedAllocator CreateForType(size_t offset = 0)
	{
		return FastFixedAlignedAllocator(sizeof(T), alignof(T), offset);
	}

	FastFixedAlignedAllocator(size_t element_size, size_t alignment, size_t offset = 0) : Alignment(alignment)
	{
		Init(element_size, alignment, offset);
	}

	void Init(size_t element_size, size_t alignment, size_t offset = 0)
	{
		// pad the element size for alignment
		size_t adjusted_size = (element_size + (alignment - 1)) & (-intptr_t(alignment));

		FastFixedAllocator::Init(adjusted_size);
		this->Alignment = alignment;
		this->Offset = offset & (alignment - 1);    // offset larger than alignment makes no sense
		assert((alignment & (alignment - 1)) == 0); // only power of two alignments
		assert(alignment <= 4096);                  // no super huge alignments
	}

	virtual void Grow() override
	{
#ifndef DISABLE_CUSTOM_ALLOCATORS
		Chunk* n = new Chunk;
		Chunks.Push(n);

#ifdef FASTALLOC_STATS
		_InterlockedExchangeAdd((long*)& TotalHeapSize, sizeof(Chunk));
#endif

		char* element = n->Memory;

		uintptr_t aligned_address = uintptr_t(element + (Alignment - 1_sz)) & (-intptr_t(Alignment));
		element = (char*)aligned_address - Offset;
		if (element < n->Memory)
			element += Alignment;

		assert(element + ElementSize <= n->Memory + Chunk::size);

		while (element + ElementSize <= n->Memory + Chunk::size)
		{
			assert((uintptr_t(element + Offset) & (Alignment - 1)) == 0);
			Link* link = (Link*)element;
			link->Next = nullptr;
			FreeList.Push(link);
			element += ElementSize;
		}
#endif
	}
};

class FastAllocatorGeneral: public SystemAllocatedClass
{
protected:
	enum : size_t
	{	
		NUM_ALLOCATORS = 11, 
#if defined(_M_IX86)
		MIN_ALLOC_BIT = 3,
#elif defined(_M_X64)
		MIN_ALLOC_BIT = 4,
#endif
		MIN_ALLOC_SIZE = 1 << MIN_ALLOC_BIT,
		MAX_ALLOC_SIZE = 1 << (NUM_ALLOCATORS + MIN_ALLOC_BIT - 1),
	};

	size_t GetAllocatorIndex(size_t input) const;

	FastFixedAllocator allocators[NUM_ALLOCATORS];
#ifdef FASTALLOC_STATS
	size_t ActualMemoryUsage;
	unsigned long ExternalMemoryUsage;
	unsigned long ExternalMemoryCount;
#endif
public:
	static FastAllocatorGeneral* Get_Allocator();
	FastAllocatorGeneral();

	DECLSPEC_RESTRICT void*	Allocate(size_t size);
	DECLSPEC_RESTRICT void*	Allocate(size_t size, size_t align);
	void	Free(void* memory);
	void	Free(void* memory, size_t align);
	DECLSPEC_RESTRICT void*	Reallocate(void* memory, size_t newSize);
	size_t	GetAllocationSize(void* memory) const;
#ifdef FASTALLOC_STATS
	size_t GetTotalMemoryUsage() const { return ActualMemoryUsage; };
#endif
};

inline FastAllocatorGeneral::FastAllocatorGeneral()
#ifdef FASTALLOC_STATS
:
ActualMemoryUsage(0), 
ExternalMemoryUsage(0), 
ExternalMemoryCount(0)
#endif
{
	for (size_t i = 0; i < NUM_ALLOCATORS; ++i)
	{
		allocators[i].Init(1_sz << (i + MIN_ALLOC_BIT));
		allocators[i].Grow(); // let's do the first growing here so that we don't fight over allocating
	};
};

inline size_t FastAllocatorGeneral::GetAllocatorIndex(size_t input) const
{
	DWORD result;
	ASSUME((input & 0xFFFFFFFF00000000) == 0); // assume we are only doing allocation below 32 bits, please?
	ASSUME(input != 0);
	_BitScanReverse(&result, ((int32)(input - 1) >> (MIN_ALLOC_BIT - 1)) | 1);
	return (size_t)result;
};

DECLSPEC_RESTRICT inline void* FastAllocatorGeneral::Allocate(size_t size)
{
#ifdef DISABLE_CUSTOM_ALLOCATORS
	return malloc(size);
#else
	void* memory;

	// Let's allocate a few extra bytes so we can store either the size of the allocation or the allocator index.
	// FastFixedAllocator will always return memory aligned to exactly PLATFORM_MEMORY_ALIGNMENT due to the current Chunk layout.
	// Unfortunately that means we need to allocate an extra PLATFORM_MEMORY_ALIGNMENT bytes so we don't misalign the address we return.
	size += PLATFORM_MEMORY_ALIGNMENT;

	size_t index;

	if (size <= MAX_ALLOC_SIZE)
	{
		index = GetAllocatorIndex(size);
		memory = allocators[index].Allocate();
#ifdef FASTALLOC_STATS
		_InterlockedExchangeAdd((long*)&ActualMemoryUsage, (long)allocators[index].GetSize());
#endif
	}
	else
	{
#ifdef FASTALLOC_STATS
		_InterlockedExchangeAdd((long*)&ActualMemoryUsage, size);
		_InterlockedExchangeAdd((long*)&ExternalMemoryUsage, size);
		_InterlockedIncrement((long*)&ExternalMemoryCount);
#endif
		// Hello mister, this memory is too big. Can you carry it for me?
		memory = SystemAllocate(size);
		index = size;
	};
	
	*((size_t*)memory) = index; // save the allocator index so we can use it later to liberate the memory and not royally screw things up
	memory = (char*)memory + PLATFORM_MEMORY_ALIGNMENT;

	assert((uintptr_t(memory) & (PLATFORM_MEMORY_ALIGNMENT - 1)) == 0);

	return memory; // return actually usable memory to user
#endif
};

DECLSPEC_RESTRICT inline void* FastAllocatorGeneral::Allocate(size_t size, size_t align)
{
#ifdef DISABLE_CUSTOM_ALLOCATORS
	return _aligned_malloc(size, align);
#else
	assert(align > PLATFORM_MEMORY_ALIGNMENT);
	assert(align <= 4096); // no super large alignments
	assert((align & (align - 1)) == 0); // only power of two alignments

	// For FastFixedAllocator, we only need the index of allocator.
	// For system allocated memory, we also need the offset from the original allocation since it's variable.
	constexpr size_t metadata_size = 2 * sizeof(size_t);

	static_assert(metadata_size < 2 * PLATFORM_MEMORY_ALIGNMENT); // ensure we can always hide the metadata within one alignment

#ifdef DEBUG
	size_t original_size = size;
#endif

	if (size < align)
		size = align;

	// Our internal FastFixedAllocators have chunks that are allocated using VirtualAlloc.
	// They return addresses with sizeof(TSLNode) + (i * ElementSize) offsets within those chunks.
	// This means they always return memory misaligned by exactly sizeof(TSLNode) as long as align <= page size (or more accurately GetSystemInfo -> dwAllocationGranularity).

	size_t fixed_offset = (align - offsetof(FastFixedAllocator::Chunk, Memory));
	assert(fixed_offset < align && fixed_offset >= metadata_size);

	if ((size + fixed_offset) <= MAX_ALLOC_SIZE) {
		size += fixed_offset;
	} else {
		// For the system allocator, we need at least (align - 1) extra space to guarantee the ability to realign the returned address for the object.
		// The offset will vary depending on the system allocator's return value and the alignment.
		size += metadata_size + (align - 1);
	}

	void* memory;
	void* aligned_mem;
	size_t index;

	if (size <= MAX_ALLOC_SIZE)
	{
		index = GetAllocatorIndex(size);
		memory = allocators[index].Allocate();
		// FastFixedAllocators are guaranteed to give us overaligned memory, so this offset calculation is easy!
		aligned_mem = (char*)memory + fixed_offset;

		assert((1_sz << (index + MIN_ALLOC_BIT)) - fixed_offset >= original_size);

#ifdef FASTALLOC_STATS
		_InterlockedExchangeAdd((long*)& ActualMemoryUsage, (long)allocators[index].GetSize());
#endif
	}
	else
	{
#ifdef FASTALLOC_STATS
		_InterlockedExchangeAdd((long*)& ActualMemoryUsage, size);
		_InterlockedExchangeAdd((long*)& ExternalMemoryUsage, size);
		_InterlockedIncrement((long*)& ExternalMemoryCount);
#endif
		memory = SystemAllocate(size);
		index = size;

		uintptr_t offset = align - (uintptr_t(memory) & (align - 1));
		if (offset < metadata_size)
			offset += align;

		aligned_mem = (char*)memory + offset;
		*((size_t*)aligned_mem - 2) = offset; // save the alignment offset so we can get the original memory address back

		assert(size - offset >= original_size);
	};

	assert((uintptr_t(aligned_mem) & (align - 1)) == 0);

	*((size_t*)aligned_mem - 1) = index; // save the allocator index so we know which one to use for freeing
	return aligned_mem; // returned aligned memory to user
#endif
}


inline void FastAllocatorGeneral::Free(void* _memory)
{
#ifdef DISABLE_CUSTOM_ALLOCATORS
	return free(_memory);
#else
	if (_memory)
	{
		void* memory = (char*)_memory - PLATFORM_MEMORY_ALIGNMENT; // Retrieve the real memory
		
		size_t index = *((size_t*)memory);

		if (index < NUM_ALLOCATORS)
		{
#ifdef FASTALLOC_STATS
			_InterlockedExchangeAdd((long*)&ActualMemoryUsage, -(long)allocators[index].GetSize());
#endif
			allocators[index].Free(memory);
		}
		else
		{
#ifdef FASTALLOC_STATS
			_InterlockedExchangeAdd((long*)&ActualMemoryUsage, -(long)index);
			_InterlockedExchangeAdd((long*)&ExternalMemoryUsage, -(long)index);
			_InterlockedDecrement((long*)&ExternalMemoryCount);
#endif
			// Mister, can you set this memory free for me?
			SystemFree(memory);
		}
	}
#endif
};

inline void FastAllocatorGeneral::Free(void* _memory, size_t align)
{
#ifdef DISABLE_CUSTOM_ALLOCATORS
	(void)align;
	return _aligned_free(_memory);
#else
	if (_memory)
	{
		size_t index = *((size_t*)_memory - 1);

		if (index < NUM_ALLOCATORS)
		{
#ifdef FASTALLOC_STATS
			_InterlockedExchangeAdd((long*)& ActualMemoryUsage, -(long)allocators[index].GetSize());
#endif
			// original memory is always misaligned by sizeof(TSLNode) bytes, thus the offset only depends on align
			size_t fixed_offset = (align - offsetof(FastFixedAllocator::Chunk, Memory));
			void* memory = (char*)_memory - fixed_offset;
			allocators[index].Free(memory);
		}
		else
		{
#ifdef FASTALLOC_STATS
			_InterlockedExchangeAdd((long*)& ActualMemoryUsage, -(long)index);
			_InterlockedExchangeAdd((long*)& ExternalMemoryUsage, -(long)index);
			_InterlockedDecrement((long*)& ExternalMemoryCount);
#endif
			// get the offset from the metadata
			size_t offset = *((size_t*)_memory - 2);
			void* memory = (char*)_memory - offset;
			SystemFree(memory);
		}
	}
#endif
}

DECLSPEC_RESTRICT inline void* FastAllocatorGeneral::Reallocate(void* _memory, size_t newSize)
{
#ifdef DISABLE_CUSTOM_ALLOCATORS
	return realloc(_memory, newSize);
#else
	if (_memory)
	{
		void* memory = (char*)_memory - PLATFORM_MEMORY_ALIGNMENT; // Retrieve the real memory
		
		size_t index = *((size_t*)memory);
		
		if(newSize > allocators[index].GetSize() - sizeof(size_t))
		{
			void* newMemory = Allocate(newSize);
			memcpy(newMemory, _memory, allocators[index].GetSize() - sizeof(size_t));
			allocators[index].Free(memory);
			_memory = newMemory;
		}
	}
	else
	{
		_memory = Allocate(newSize);
	}
	return _memory;
#endif
};

inline size_t FastAllocatorGeneral::GetAllocationSize(void* memory) const
{
	size_t index = *(((size_t*)memory) - 1);
	return (index < NUM_ALLOCATORS) ? (1_sz << (index + MIN_ALLOC_BIT)) : index;
};

//TODOx: AutoObjectPool
POP_MEMORY_MACROS
#endif
