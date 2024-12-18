#pragma once

#ifdef TL_ENABLE_PROFILE

#include "optick.h"

#define PROFILE_STARTUP()
#define PROFILE_SHUTDOWN() OPTICK_SHUTDOWN()
#define PROFILE_FUNC(...) OPTICK_EVENT(__VA_ARGS__)
#define PROFILE_FRAME(name) OPTICK_FRAME(name)
#define PROFILE_SECTION_BEGIN(name) OPTICK_PUSH(name)
#define PROFILE_SECTION_END() OPTICK_POP()

#else

#define PROFILE_STARTUP()
#define PROFILE_SHUTDOWN()
#define PROFILE_FUNC(...)
#define PROFILE_FRAME(name)
#define PROFILE_SECTION_BEGIN(name)
#define PROFILE_SECTION_END(name)

#endif
