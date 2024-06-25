#include "general.h"
#include "FastAllocator.h"
#pragma warning(push)
#pragma warning(disable: 4073) // warning C4073: initializers put in library initialization area
#pragma init_seg(lib) // leet hax to insure our threads and friends are initialized before everybody else
#pragma warning(pop)
PUSH_MEMORY_MACROS
#undef new
#undef delete
static FastAllocatorGeneral* __general_allocator = new FastAllocatorGeneral();
POP_MEMORY_MACROS

FastAllocatorGeneral* FastAllocatorGeneral::Get_Allocator()
{
	return __general_allocator;	
};