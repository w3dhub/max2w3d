#ifndef _PLATFORM_WIN32_H_
#define _PLATFORM_WIN32_H_
#ifndef WIN32
#error This file can only be included for Win32/64 builds for now.
#endif
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES // message types WM_*
#define NOWINSTYLES   // window styles WS_*
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW // SW_*
//#define OEMRESOURCE // includes various image/GUI related stuff if defined
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR // Control and Dialog routines
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
//#define NOUSER // all the window-related and lots of other stuff
#define NONLS  // WideCharToMultiByte/MultiByteToWideChar, CP_UTF8, GetACP()
//#define NOMB   // MessageBox
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG // PeekMessage, etc.
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC // typedef TEXTMETRIC and associated routines
#define NOWH
#define NOWINOFFSETS // GWL_*, GCL_*, associated routines
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#include <windows.h>

#ifndef PLATFORM_NOTYPES
typedef __int16				int16;
typedef unsigned __int16	uint16;
typedef __int32				int32;
typedef unsigned __int32	uint32;
typedef __int64				int64;
typedef unsigned __int64	uint64;
typedef unsigned int		uint;
typedef void*				handle_t;
#endif

//size_t literal
constexpr size_t operator"" _sz(unsigned long long v) { return static_cast<size_t>(v); }

#if defined (_M_IX86)
#define PLATFORM_32_BIT 1
#undef PLATFORM_64_BIT
#define PLATFORM_MEMORY_ALIGNMENT 8
#elif defined (_M_X64)
#undef PLATFORM_32_BIT
#define PLATFORM_64_BIT 1
#define PLATFORM_MEMORY_ALIGNMENT 16
#endif

inline void YieldThread() { YieldProcessor(); };
#define DECLSPEC_ALIGN(alignment)	__declspec(align(alignment))
#define DECLSPEC_RESTRICT			__declspec(restrict)

#ifdef NDEBUG
#define ASSUME(_Expression) __assume(_Expression);
#else
#define ASSUME(_Expression) assert(_Expression);
#endif

#endif
