#ifndef _MEMORYMANAGER_H_
#define _MEMORYMANAGER_H_

#include "platform.h"

//#define USING_NEW_OVERRIDE
//#define USING_DELETE_OVERRIDE
//#define DECLARE_PLACEMENT_NEW
//#define DETAILED_LEAK_LOGS 1
//#define LARGE_ALLOCATION_DEBUG_STATS
//#define ALWAYS_WIPE_MEMORY
//#define DISABLE_CUSTOM_ALLOCATORS

#if PLATFORM_64_BIT && DEBUG
#define MEMORY_64BIT_ALLOCATION_DEBUG // Makes accidental pointer truncations from 64 to 32 bits much more likely to trigger bugs. Allocation of chunks is slower.
#endif

// we need std::align_val_t for disambiguating new(ptr, size) from new(ptr, align)
#include <new>

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

#define MEMORYMANAGER_DEBUGCONFIGURATION	0xbaadc0de
#define MEMORYMANAGER_RELEASECONFIGURATION	0xdeadbeef

#ifndef NDEBUG
#define MEMORYMANAGER_CONFIGURATION MEMORYMANAGER_DEBUGCONFIGURATION
#else
#define MEMORYMANAGER_CONFIGURATION MEMORYMANAGER_RELEASECONFIGURATION
#endif

void	CheckMemoryManagerConfiguration(int32 config);	// Call this function with MEMORYMANAGER_CONFIGURATION as the value
void	SetThreadTrackingInformation(const char* file, unsigned int line, const char* function);
void	PushAllocationTag(const char* tag);
void	PopAllocationTag();

class MemoryManagerTagger
{
public:
	MemoryManagerTagger(const char* tag) { PushAllocationTag(tag); };
	~MemoryManagerTagger() { PopAllocationTag(); };
private:
	/* Not implemented */
	MemoryManagerTagger(const MemoryManagerTagger&);
	MemoryManagerTagger operator=(const MemoryManagerTagger&);
};

#ifndef NDEBUG
#define MMGR_TAG(tag) MemoryManagerTagger mmgr_tag(tag);
#else
#define MMGR_TAG(tag) 
#endif

[[nodiscard]] DECLSPEC_RESTRICT void* AllocateMemory(size_t size);
[[nodiscard]] DECLSPEC_RESTRICT void* AllocateMemory(size_t size, AllocType type, const char* file, unsigned int line, const char* function);
[[nodiscard]] DECLSPEC_RESTRICT void* ReallocateMemory(void* memory, size_t new_size);
[[nodiscard]] DECLSPEC_RESTRICT void* ReallocateMemory(void* memory, size_t new_size, const char* file, unsigned int line, const char* function);
void	FreeMemory(void* memory);
void	FreeMemory(void* memory, DeallocType type, const char* file, unsigned int line, const char* function);
size_t	GetMemorySize(void* memory);

[[nodiscard]] void* operator new(size_t size);
[[nodiscard]] void* operator new(size_t size, std::align_val_t align);
void operator delete(void* memory);
void operator delete(void* memory, std::align_val_t align);
void operator delete(void* memory, size_t size);
void operator delete(void* memory, size_t size, std::align_val_t align);

[[nodiscard]] void* operator new[](size_t size);
[[nodiscard]] void* operator new[](size_t size, std::align_val_t align);
void operator delete[](void* memory);
void operator delete[](void* memory, std::align_val_t align);
void operator delete[](void* memory, size_t size);
void operator delete[](void* memory, size_t size, std::align_val_t align);

#ifdef DECLARE_PLACEMENT_NEW
// TODO:
// - according to https://en.cppreference.com/w/cpp/memory/new/operator_new you can't replace these new functions, especially not declared as inline. Maybe MSVC-specific?
// - add aligned placement new
inline void* operator new(size_t size, void *location) throw()
{
#ifndef NDEBUG
	SetThreadTrackingInformation(nullptr, 0, nullptr);
#endif
	return location;
};

inline void* operator new[](size_t size, void *location) throw()
{
#ifndef NDEBUG
	SetThreadTrackingInformation(nullptr, 0, nullptr);
#endif
	return location;
};
#endif

#define PUSH_MEMORY_MACROS	__pragma(push_macro("new")) __pragma(push_macro("delete")) 
#define POP_MEMORY_MACROS	__pragma(pop_macro("new")) __pragma(pop_macro("delete"))
	
#ifndef NDEBUG
#ifdef USING_NEW_OVERRIDE 
#define new			(SetThreadTrackingInformation(__FILE__, __LINE__, __FUNCTION__), false) ? (assert(false), nullptr) : new
#endif
#ifdef USING_DELETE_OVERRIDE
#define	delete		(SetThreadTrackingInformation(__FILE__, __LINE__, __FUNCTION__), false) ? assert(false) : delete
#endif
#endif

#endif