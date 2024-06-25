#ifndef TT_INCLUDE__DEFINES_H
#define TT_INCLUDE__DEFINES_H
#ifdef DEBUG
#ifndef _DEBUG
#error debug
#endif
#endif

#ifndef W3D_MAX_TOOLS
#define _HAS_EXCEPTIONS 0
#endif

#define ENABLE_VECTOR_RANGE_CHECKS 0

// put global defines here
#include <stdarg.h>
#include <xmemory> // NOTE: this is here so that the "byte" typedef below doesn't cause billions of errors

// shortcuts to avoid typing two characters...
typedef uint64_t uint64;
typedef int64_t sint64;
typedef uint32_t uint32;
typedef int32_t sint32;
typedef uint16_t uint16;
typedef int16_t sint16;
typedef uint8_t uint8;
typedef int8_t sint8;
typedef uint8 byte;
typedef sint32 sint;
typedef uint32 uint;

#if (W3D_MAX_TOOLS) || (WWCONFIG) || (TDBEDIT) || (W3DSHADER) || (W3DLIB_EXPORTS) || (W3DMESHMENDER) || (W3DDEPENDS) || (W3DMAPPER) || (ACHASH) || (PACKAGEEDITOR) || (FIXPLANES) || (MERGELOD) || (MIXCHECK) || (MAKEMIX) || (ALTMAT) || (CHUNKDUMP)
#define EXTERNAL 1
#endif

#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4091) // 'keyword' : ignored on left of 'type' when no variable is declared
#pragma warning(disable: 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable: 4505) // unreferenced local function has been removed
#pragma warning(disable: 6509) // warning c6509: Return used on precondition
#pragma warning(disable: 4351) //warning C4351: new behavior: elements of array 'x' will be default initialized
#pragma warning(disable: 4324) //warning C4324: structure was padded due to alignment specifier

//class needs to have dll-interface to used by clients of class. Except that it doesn't. 
//If it did, the linker would complain.
#pragma warning(disable:4251)

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601
#define _USE_MATH_DEFINES
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define SAFE_DELETE_ARRAY(p)	{ delete[] p; p = nullptr; }
#define SAFE_DELETE(p)			{ delete p; p = nullptr; }

// breakpoint
#if defined(_M_IX86)
#define TT_INTERRUPT  { __asm { __asm int 3 } }
#elif defined(_M_AMD64)
#define TT_INTERRUPT  { __debugbreak(); terminate(); }
#endif

// assert that is not removed in release mode
#define TT_RELEASE_ASSERT(expression) { __analysis_assume(expression); if (!(expression)) TT_INTERRUPT; }

// assert
#ifdef DEBUG
#	define TT_ASSERT(expression) \
   {                             \
	   __analysis_assume(expression); \
      if (!(expression))         \
         TT_INTERRUPT;            \
   }
#else
#	define TT_ASSERT(expression) \
   {                             \
	   __analysis_assume(expression); \
   }
#endif

// assumption optimization
#ifdef DEBUG
#	define TT_ASSUME(x) TT_ASSERT(x)
#elif defined(NDEBUG) && defined(_MSC_VER)
#	define TT_ASSUME(x) __assume(x)
#else
#	define TT_ASSUME(x)
#endif

// unreachable code
#	ifdef DEBUG
#		define TT_UNREACHABLE { TT_INTERRUPT }
#	else
#		define TT_UNREACHABLE __assume(false);
#	endif

#define TT_UNIMPLEMENTED TT_INTERRUPT
#define TT_UNTESTED TT_INTERRUPT

// deprecated code
#define TT_DEPRECATED(x) __declspec(deprecated(x))

// inlined function calls
#if defined(NDEBUG) && defined(_MSC_VER)
#	define TT_INLINE __forceinline
#else
#	define TT_INLINE inline
#endif

#ifndef SSGMPLUGIN
bool IsGameClient();
bool IsDedicatedServer();
#endif
#ifdef EXTERNAL
TT_INLINE bool IsGameClient() { return false; }
TT_INLINE bool IsDedicatedServer() { return false; }
#endif

// to communicate with Renegade
#ifdef SHARED_EXPORTS
#   define SHARED_API __declspec(dllexport)
#else
#if (EXTERNAL)
#	define SHARED_API
#else
#   define SHARED_API __declspec(dllimport)
#endif
#endif

#ifdef SSGM
#   define SCRIPTS_API __declspec(dllexport)
#else
#ifdef SSGMPLUGIN
#   define SCRIPTS_API __declspec(dllimport)
#else
#   define SCRIPTS_API
#endif
#endif

template<int stackBufferLength, typename Char> class FormattedString;

template<int stackBufferLength> class FormattedString<stackBufferLength, char>
{

public:

	char stackBuffer[stackBufferLength+1];
	char* heapBuffer;
	const char* value;
	int length;

	FormattedString(_Printf_format_string_ const char* format, ...)
	{
		va_list arguments;
		va_start(arguments, format);
		length = vsnprintf(stackBuffer, stackBufferLength, format, arguments);
		if (length >= 0)
		{
			// The formatted string fit on the stack. Use the stack buffer.
			stackBuffer[length] = '\0'; // Fix terminator. Only necessary if length == stackBufferLength.
			heapBuffer = nullptr;
			value = stackBuffer;
		}
		else
		{
			// The formatted string did not fit on the stack. Allocate a heap buffer.
			length = _vscprintf(format, arguments);
			heapBuffer = new char[length + 1];
			vsprintf(heapBuffer, format, arguments);
			value = heapBuffer;
		}
		va_end(arguments);
	}

	~FormattedString()
	{
		delete[] heapBuffer;
	}
	
	const char* getValue() const { return value; }
	int getLength() const { return length; }
};

template<int stackBufferLength> class FormattedString<stackBufferLength, wchar_t>
{

public:

	wchar_t stackBuffer[stackBufferLength+1];
	wchar_t* heapBuffer;
	const wchar_t* value;
	int length;

	FormattedString(_Printf_format_string_ const wchar_t* format, ...)
	{
		va_list arguments;
		va_start(arguments, format);
		length = _vsnwprintf(stackBuffer, stackBufferLength, format, arguments);
		if (length >= 0)
		{
			// The formatted string fit on the stack. Use the stack buffer.
			stackBuffer[length] = '\0'; // Fix terminator. Only necessary if length == stackBufferLength.
			heapBuffer = nullptr;
			value = stackBuffer;
		}
		else
		{
			// The formatted string did not fit on the stack. Allocate a heap buffer.
			length = _vscwprintf(format, arguments);
			heapBuffer = new wchar_t[length + 1];
			_vsnwprintf(heapBuffer, length + 1, format, arguments);
			value = heapBuffer;
		}
		va_end(arguments);
	}

	~FormattedString()
	{
		delete[] heapBuffer;
	}
	
	const wchar_t* getValue() const { return value; }
	int getLength() const { return length; }
};

// Format a string (sprintf style) using a stack buffer of a given size if possible, or a heap buffer otherwise.
#define TT_FORMAT(maxFormattedLength, format, ...) FormattedString<maxFormattedLength, char>(format, __VA_ARGS__).getValue()
#define TT_FORMAT_WIDE(maxFormattedLength, format, ...) FormattedString<maxFormattedLength, wchar_t>(format, __VA_ARGS__).getValue()

// Define the possible values for the Exe variable, rather than scattering magic numbers throughout the code
#define EXE_CLIENT 0
#define EXE_SERVER 1
#define EXE_LEVELEDIT 4
#define EXE_UNINITIALISED 6

// Thanks to the magic of variadic macros, you can put any kind of code content in this and it'll work the same as an ifdef.
// Useful for function params
#ifdef DEBUG
#define DEBUG_ONLY(...) __VA_ARGS__
#else
#define DEBUG_ONLY(...)
#endif

// NOTE(Dgh): So far only the server and parts of the editor are vetted. Only enable on client or editor if you know what you're doing.
#define ENABLE_FP_EXCEPTIONS 0

#if ENABLE_FP_EXCEPTIONS
// Declare an object of this type in a scope in order to suppress
// all floating-point exceptions temporarily. The old exception
// state will be reset at the end.
class FPExceptionDisabler
{
public:
    FPExceptionDisabler()
    {
        // Retrieve the current state of the exception flags. This
        // must be done before changing them. _MCW_EM is a bit
        // mask representing all available exception masks.
        // Fixed - used to pass _MCW_EM for the last two values.
        _controlfp_s(&mOldValues, 0, 0);
        // Set all of the exception flags, which suppresses FP
        // exceptions on the x87 and SSE units.
        _controlfp_s(0, _MCW_EM, _MCW_EM);
    }
    ~FPExceptionDisabler()
    {
        // Clear any pending FP exceptions. This must be done
        // prior to enabling FP exceptions since otherwise there
        // may be a 'deferred crash' as soon the exceptions are
        // enabled.
        _clearfp();

        // Reset (possibly enabling) the exception status.
        _controlfp_s(0, mOldValues, _MCW_EM);
    }

private:
    unsigned int mOldValues;

    // Make the copy constructor and assignment operator private
    // and unimplemented to prohibit copying.
    FPExceptionDisabler(const FPExceptionDisabler&);
    FPExceptionDisabler& operator=(const FPExceptionDisabler&);
};

// Declare an object of this type in a scope in order to enable a
// specified set of floating-point exceptions temporarily. The old
// exception state will be reset at the end.
// This class can be nested.
class FPExceptionEnabler
{
public:
    // Overflow, divide-by-zero, and invalid-operation are the FP
    // exceptions most frequently associated with bugs.
    FPExceptionEnabler(unsigned int enableBits = _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID)
    {
        // Retrieve the current state of the exception flags. This
        // must be done before changing them. _MCW_EM is a bit
        // mask representing all available exception masks.
        _controlfp_s(&mOldValues, _MCW_EM, _MCW_EM);

        // Make sure no non-exception flags have been specified,
        // to avoid accidental changing of rounding modes, etc.
        enableBits &= _MCW_EM;

        // Clear any pending FP exceptions. This must be done
        // prior to enabling FP exceptions since otherwise there
        // may be a 'deferred crash' as soon the exceptions are
        // enabled.
        _clearfp();

        // Zero out the specified bits, leaving other bits alone.
        _controlfp_s(0, ~enableBits, enableBits);
    }
    ~FPExceptionEnabler()
    {
        // Reset the exception state.
        _controlfp_s(0, mOldValues, _MCW_EM);
    }

private:
    unsigned int mOldValues;

    // Make the copy constructor and assignment operator private
    // and unimplemented to prohibit copying.
    FPExceptionEnabler(const FPExceptionEnabler&);
    FPExceptionEnabler& operator=(const FPExceptionEnabler&);
};
#else
// NOTE(Dgh): [[maybe_unused]] doesn't work to disable C4101 warning even though it should... adding an empty constructor for now.
struct [[maybe_unused]] FPExceptionEnabler { FPExceptionEnabler() {}; };
struct [[maybe_unused]] FPExceptionDisabler { FPExceptionDisabler() {}; };
#endif


// Go-style "defer", executes code at the end of the scope
template <typename F>
struct DeferImpl {
	F f;
	DeferImpl(F f) : f(f) {}
	~DeferImpl() { f(); }
};

template <typename F>
DeferImpl<F> Defer_Func(F f) {
	return DeferImpl<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define DEFER(code)   auto DEFER_3(_defer_) = Defer_Func([&](){code;})


#endif
