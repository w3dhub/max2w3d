#include "general.h"
#include "platform.h"
#include "MemTracker.h"
#include <intrin.h>

#ifdef MEMORY_64BIT_ALLOCATION_DEBUG
#include <stdio.h>

// https://randomascii.wordpress.com/2012/02/14/64-bit-made-easy/
void ReserveBottomMemory()
{
	static bool s_initialized = false;
	if (s_initialized)
		return;
	s_initialized = true;

	// Start by reserving large blocks of address space, and then
	// gradually reduce the size in order to capture all of the
	// fragments. Technically we should continue down to 64 KB but
	// stopping at 1 MB is sufficient to keep most allocators out.

	const size_t LOW_MEM_LINE = 0x100000000LL;
	size_t totalReservation = 0;
	size_t numVAllocs = 0;
	size_t numHeapAllocs = 0;
	size_t oneMB = 1024 * 1024;
	for (size_t size = 256 * oneMB; size >= oneMB; size /= 2)
	{
		for (;;)
		{
			void* p = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
			if (!p)
				break;

			if ((size_t)p >= LOW_MEM_LINE)
			{
				// We don't need this memory, so release it completely.
				VirtualFree(p, 0, MEM_RELEASE);
				break;
			}

			totalReservation += size;
			++numVAllocs;
		}
	}

	// Now repeat the same process but making heap allocations, to use up
	// the already reserved heap blocks that are below the 4 GB line.
	HANDLE heap = GetProcessHeap();
	for (size_t blockSize = 64 * 1024; blockSize >= 16; blockSize /= 2)
	{
		for (;;)
		{
			void* p = HeapAlloc(heap, 0, blockSize);
			if (!p)
				break;

			if ((size_t)p >= LOW_MEM_LINE)
			{
				// We don't need this memory, so release it completely.
				HeapFree(heap, 0, p);
				break;
			}

			totalReservation += blockSize;
			++numHeapAllocs;
		}
	}

	// Perversely enough the CRT doesn't use the process heap. Suck up
	// the memory the CRT heap has already reserved.
	for (size_t blockSize = 64 * 1024; blockSize >= 16; blockSize /= 2)
	{
		for (;;)
		{
			void* p = malloc(blockSize);
			if (!p)
				break;

			if ((size_t)p >= LOW_MEM_LINE)
			{
				// We don't need this memory, so release it completely.
				free(p);
				break;
			}

			totalReservation += blockSize;
			++numHeapAllocs;
		}
	}

	// Print diagnostics showing how many allocations we had to make in
	// order to reserve all of low memory, typically less than 200.
	char buffer[1000];
	sprintf_s(buffer, "Reserved %1.3f MB (%d vallocs,"
		"%d heap allocs) of low-memory.\n",
		totalReservation / (1024 * 1024.0),
		(int)numVAllocs, (int)numHeapAllocs);
	OutputDebugStringA(buffer);
}
#endif

DWORD TLSIndex = TLS_OUT_OF_INDEXES;
BOOL WINAPI DllMain(HINSTANCE /*instance*/, DWORD reason, LPVOID /*reserved*/)
{
	_CrtSetDebugFillThreshold(0); // removes unnecessary performance hog, you can't pass the wrong size if you use the templated versions.

#ifdef MEMORY_64BIT_ALLOCATION_DEBUG
	// Try to reserve the bottom 32 bits of address space to trigger pointer-to-int truncation bugs more easily.
	ReserveBottomMemory();
#endif

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

	switch (reason) 
	{ 
	case DLL_PROCESS_ATTACH: 
		// Allocate a TLS index.
		TLSIndex = TlsAlloc();
		if (TLSIndex == TLS_OUT_OF_INDEXES)
		{
			// Uhoh.
			return false;
		}
		// No break: Initialize the index for first thread.
	case DLL_THREAD_ATTACH: 
		// Initialize the TLS index for this thread.
		{
			void* tls_data = calloc(1, sizeof(MemoryTrackerThreadLocalInformation));
			if (tls_data != nullptr)
			{
				TlsSetValue(TLSIndex, tls_data);
			};
		}
		break; 
	case DLL_THREAD_DETACH: 
		// Release the allocated memory for this thread.
		{
			void* tls_data = TlsGetValue(TLSIndex);
			if (tls_data != nullptr)
			{
				free(tls_data);
			};
		}
		break; 
	case DLL_PROCESS_DETACH: 
		// Release the allocated memory for this thread.
		void* tls_data = TlsGetValue(TLSIndex);
		if (tls_data != nullptr)
		{
			free(tls_data);
		};

		// Release the TLS index.
		TlsFree(TLSIndex); 
		break; 
	} 

	return true; 
};