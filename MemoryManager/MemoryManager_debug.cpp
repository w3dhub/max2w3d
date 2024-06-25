#include "general.h"
#include "platform.h"
#include "MemoryManager.h"
#include "MemTracker.h"

PUSH_MEMORY_MACROS
#undef new
#undef delete

void CheckMemoryManagerConfiguration(int32 /*config*/)
{
};

void SetThreadTrackingInformation(const char* file, unsigned int line, const char* function)
{
	MemoryTracker::SetThreadLocalInformation(file, function, line);
};

void PushAllocationTag(const char* tag)
{
	MemoryTracker::PushAllocationTag(tag);
};

void PopAllocationTag()
{
	MemoryTracker::PopAllocationTag();
};

DECLSPEC_RESTRICT void* AllocateMemory(size_t size)
{
	return MemoryTracker::Allocate(size, 0, AllocType_Malloc);
};

DECLSPEC_RESTRICT void* AllocateMemory(size_t size, AllocType type, const char* file, unsigned int line, const char* function)
{
	return MemoryTracker::Allocate(size, 0, type, file, function, line);
};

DECLSPEC_RESTRICT void* ReallocateMemory(void* memory, size_t new_size)
{
	return MemoryTracker::Reallocate(memory, new_size);
};

DECLSPEC_RESTRICT void* ReallocateMemory(void* memory, size_t new_size, const char* file, unsigned int line, const char* function)
{
	return MemoryTracker::Reallocate(memory, new_size, file, function, line);
};

void FreeMemory(void* memory)
{
	MemoryTracker::Free(memory, 0, DeallocType_Free);
};

void FreeMemory(void* memory, DeallocType type, const char* file, unsigned int line, const char* function)
{
	MemoryTracker::Free(memory, 0, type, file, function, line);
};

size_t GetMemorySize(void* memory)
{
	return MemoryTracker::GetAllocationSize(memory);
};

void* operator new(size_t size)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();
	return MemoryTracker::Allocate(size, 0, AllocType_New, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};

void* operator new(size_t size, std::align_val_t align)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();
	return MemoryTracker::Allocate(size, size_t(align), AllocType_NewAligned, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};

void operator delete(void* memory)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();	
	MemoryTracker::Free(memory, 0, DeallocType_Delete, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};

void operator delete(void* memory, std::align_val_t align)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();	
	MemoryTracker::Free(memory, size_t(align), DeallocType_DeleteAligned, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};

void operator delete(void* memory, size_t /*size*/)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();
	MemoryTracker::Free(memory, 0, DeallocType_Delete, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};

void operator delete(void* memory, size_t /*size*/, std::align_val_t align)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();
	MemoryTracker::Free(memory, size_t(align), DeallocType_DeleteAligned, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};

void* operator new[](size_t size)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();
	return MemoryTracker::Allocate(size, 0, AllocType_VectorNew, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};

void* operator new[](size_t size, std::align_val_t align)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();
	return MemoryTracker::Allocate(size, size_t(align), AllocType_VectorNewAligned, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};
void operator delete[](void* memory)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();	
	MemoryTracker::Free(memory, 0, DeallocType_VectorDelete, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};

void operator delete[](void* memory, std::align_val_t align)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();	
	MemoryTracker::Free(memory, size_t(align), DeallocType_VectorDeleteAligned, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};

void operator delete[](void* memory, size_t /*size*/)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();
	MemoryTracker::Free(memory, 0, DeallocType_VectorDelete, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};

void operator delete[](void* memory, size_t /*size*/, std::align_val_t align)
{
	MemoryTrackerThreadLocalInformation* info = MemoryTracker::GetThreadLocalInformation();
	MemoryTracker::Free(memory, size_t(align), DeallocType_VectorDeleteAligned, info->CurrentSourceFile, info->CurrentSourceFunction, info->CurrentSourceLine);
};

POP_MEMORY_MACROS;