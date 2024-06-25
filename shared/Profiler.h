#pragma once

#ifndef __TT_SHARED_PROFILER_H_INCLUDED__
#define __TT_SHARED_PROFILER_H_INCLUDED__

#ifdef TT_PROFILE
// This is just a wrapper around Optick. (https://github.com/bombomby/optick)
// Done in this way to allow us to easily replace it with different instrumentation if we need to

#define R_STATS_OPTICK_INTEGRATION 1 // feed r_stats data to Optick (TODO(Dgh): with the current architecture this is really difficult/slow, so not implemented)

#define USE_OPTICK 1
#define OPTICK_ENABLE_GPU 0 // only Vulkan and D3D12 supported
#include "optick.h"

// Delimits main loop iterations
#define TT_PROFILER_FRAME(NAME) OPTICK_FRAME(NAME)

// Delimits thread scope
#define TT_PROFILER_THREAD(NAME) OPTICK_THREAD(NAME)
#define TT_PROFILER_THREAD_START(NAME) OPTICK_START_THREAD(NAME)
#define TT_PROFILER_THREAD_STOP() OPTICK_STOP_THREAD()

// Delimits a named scope (name can be omitted, will be auto-generated then)
#define TT_PROFILER_SCOPE(...) OPTICK_EVENT(__VA_ARGS__)
#define TT_PROFILER_SCOPE_START(...) OPTICK_PUSH(__VA_ARGS__)
#define TT_PROFILER_SCOPE_STOP() OPTICK_POP()

// Named scope with a category (different colors, see Optick::Category::FOO) [use OPTICK_FUNC as name to auto-generate]
#define TT_PROFILER_CATEGORY(NAME, CATEGORY) OPTICK_CATEGORY(NAME, CATEGORY)

// Attaches data to the current scope (e.g. object name, etc.)
#define TT_PROFILER_TAG(NAME, DATA) OPTICK_TAG(NAME, DATA)

// Generating EventDescription once during initialization:
// Optick::EventDescription* description = Optick::EventDescription::CreateShared("FunctionName");
// Then we could just use a pointer to cached description later for profiling:
// OPTICK_CUSTOM_EVENT(description);
#define TT_PROFILER_EVENT(DESCRIPTION) OPTICK_CUSTOM_EVENT(DESCRIPTION)

#define TT_PROFILER_SHUTDOWN() { OPTICK_STOP_CAPTURE(); OPTICK_SHUTDOWN(); }

#define TT_PROFILER_START_CAPTURE(...) OPTICK_START_CAPTURE(__VA_ARGS__);
#define TT_PROFILER_STOP_CAPTURE() OPTICK_STOP_CAPTURE();
#define TT_PROFILER_SAVE_CAPTURE(...) OPTICK_SAVE_CAPTURE(__VA_ARGS__);

#else

#define TT_PROFILER_FRAME(NAME)
#define TT_PROFILER_THREAD(NAME)
#define TT_PROFILER_THREAD_START(NAME)
#define TT_PROFILER_THREAD_STOP()
#define TT_PROFILER_SCOPE(...)
#define TT_PROFILER_SCOPE_START(...)
#define TT_PROFILER_SCOPE_STOP()
#define TT_PROFILER_CATEGORY(NAME, CATEGORY)
#define TT_PROFILER_TAG(NAME, DATA)
#define TT_PROFILER_EVENT(DESCRIPTION)
#define TT_PROFILER_SHUTDOWN()
#define TT_PROFILER_START_CAPTURE(...)
#define TT_PROFILER_STOP_CAPTURE()
#define TT_PROFILER_SAVE_CAPTURE(...)

#endif

#endif //__TT_SHARED_PROFILER_H_INCLUDED__
