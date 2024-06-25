#ifndef TT_INCLUDE__STANDARD_H
#define TT_INCLUDE__STANDARD_H

// make windows.h smaller
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
//#define NOVIRTUALKEYCODES
//#define NOWINMESSAGES // message types WM_*
//#define NOWINSTYLES   // window styles WS_*
//#define NOSYSMETRICS
//#define NOMENUS
#define NOICONS
//#define NOKEYSTATES
//#define NOSYSCOMMANDS
//#define NORASTEROPS
//#define NOSHOWWINDOW // SW_*
//#define OEMRESOURCE // includes various image/GUI related stuff if defined
#define NOATOM
//#define NOCLIPBOARD
//#define NOCOLOR
//#define NOCTLMGR // Control and Dialog routines
//#define NODRAWTEXT
//#define NOGDI
#define NOKERNEL
//#define NOUSER // all the window-related and lots of other stuff
//#define NONLS  // WideCharToMultiByte/MultiByteToWideChar, CP_UTF8, GetACP()
//#define NOMB   // MessageBox
#define NOMEMMGR
#define NOMETAFILE
//#define NOMINMAX
//#define NOMSG // PeekMessage, etc.
#define NOOPENFILE
//#define NOSCROLL
#define NOSERVICE
#define NOSOUND
//#define NOTEXTMETRIC // typedef TEXTMETRIC and associated routines
//#define NOWH
//#define NOWINOFFSETS // GWL_*, GCL_*, associated routines
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

#include <array>
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <emmintrin.h>
#include <math.h>
#include <memory>
#include <new>
#include <process.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <vector>
#include <string>
#include <unordered_map>

#include <windows.h>

#include "MemoryManager.h"
#include "Profiler.h"

#endif
