#include "general.h"
#include "platform.h"
#include <intrin.h>

BOOL WINAPI DllMain(HINSTANCE /*instance*/, DWORD /*reason*/, LPVOID /*reserved*/)
{

#if PLATFORM_64_BIT
	// We need cmpxchg16b (128 bit compare exchange) to run our custom allocator on 64 bit. Afaik only the earliest Core 2 and Athlon 64 models are missing this instruction.
	int info[4];
	__cpuid(info, 1);
	if (!(info[2] & (1 << 13)))
	{
		MessageBox(nullptr, L"This application cannot run on your computer because the CPU is missing support for the cmpxchg16b instruction.", L"Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
		return false;
	}
#endif

	return true;
} 