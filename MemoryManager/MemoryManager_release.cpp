#include "general.h"
#include "platform.h"
#include "MemoryManager.h"
#include "FastAllocator.h"

PUSH_MEMORY_MACROS
#undef new
#undef delete

void CheckMemoryManagerConfiguration(int32 config)
{
	if (config != MEMORYMANAGER_RELEASECONFIGURATION)
	{
		MessageBox(nullptr, L"This is a friendly warning to remind you that running the release-mode memory manager with debug-mode executables is not recommended and may cause unexpected results.", L"MemoryManager.dll", MB_OK | MB_ICONWARNING);
	};
};

void SetThreadTrackingInformation(const char* /*file*/, unsigned int /*line*/, const char* /*function*/)
{
};

void PushAllocationTag(const char* /*tag*/)
{
};

void PopAllocationTag()
{
};

DECLSPEC_RESTRICT void* AllocateMemory(size_t size)
{
	return FastAllocatorGeneral::Get_Allocator()->Allocate(size);
};

DECLSPEC_RESTRICT void* AllocateMemory(size_t size, AllocType /*type*/, const char* /*file*/, unsigned int /*line*/, const char* /*function*/)
{
	return FastAllocatorGeneral::Get_Allocator()->Allocate(size);
};

DECLSPEC_RESTRICT void* ReallocateMemory(void* memory, size_t new_size)
{
	return FastAllocatorGeneral::Get_Allocator()->Reallocate(memory, new_size);
};

DECLSPEC_RESTRICT void* ReallocateMemory(void* memory, size_t new_size, const char* /*file*/, unsigned int /*line*/, const char* /*function*/)
{
	return FastAllocatorGeneral::Get_Allocator()->Reallocate(memory, new_size);
};

void FreeMemory(void* memory)
{
	FastAllocatorGeneral::Get_Allocator()->Free(memory);
};

void FreeMemory(void* memory, DeallocType /*type*/, const char* /*file*/, unsigned int /*line*/, const char* /*function*/)
{
	FastAllocatorGeneral::Get_Allocator()->Free(memory);
};

size_t GetMemorySize(void* memory)
{
	return FastAllocatorGeneral::Get_Allocator()->GetAllocationSize(memory);
};

void* operator new(size_t size)
{
	return FastAllocatorGeneral::Get_Allocator()->Allocate(size);
};

void* operator new(size_t size, std::align_val_t align)
{
	return FastAllocatorGeneral::Get_Allocator()->Allocate(size, size_t(align));
};

void operator delete(void* memory)
{
	FastAllocatorGeneral::Get_Allocator()->Free(memory);
};

void operator delete(void* memory, std::align_val_t align)
{
	FastAllocatorGeneral::Get_Allocator()->Free(memory, size_t(align));
};

void operator delete(void* memory, size_t /*size*/)
{
	FastAllocatorGeneral::Get_Allocator()->Free(memory);
};

void operator delete(void* memory, size_t /*size*/, std::align_val_t align)
{
	FastAllocatorGeneral::Get_Allocator()->Free(memory, size_t(align));
};

void* operator new[](size_t size)
{
	return FastAllocatorGeneral::Get_Allocator()->Allocate(size);
};

void* operator new[](size_t size, std::align_val_t align)
{
	return FastAllocatorGeneral::Get_Allocator()->Allocate(size, size_t(align));
};

void operator delete[](void* memory)
{
	FastAllocatorGeneral::Get_Allocator()->Free(memory);
};

void operator delete[](void* memory, std::align_val_t align)
{
	FastAllocatorGeneral::Get_Allocator()->Free(memory, size_t(align));
};

void operator delete[](void* memory, size_t /*size*/)
{
	FastAllocatorGeneral::Get_Allocator()->Free(memory);
};

void operator delete[](void* memory, size_t /*size*/, std::align_val_t align)
{
	FastAllocatorGeneral::Get_Allocator()->Free(memory, size_t(align));
};

POP_MEMORY_MACROS;