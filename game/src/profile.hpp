#pragma once

#ifdef TL_ENABLE_PROFILE

#include "optick.h"

#define PROFILE_STARTUP()
#define PROFILE_SHUTDOWN() OPTICK_SHUTDOWN()
#define PROFILE_FUNC(...) OPTICK_EVENT(__VA_ARGS__)
#define PROFILE_FRAME(name) OPTICK_FRAME(name)

#else

#define PROFILE_STARTUP()
#define PROFILE_SHUTDOWN()
#define PROFILE_FUNC(...)
#define PROFILE_FRAME(name)

#endif
