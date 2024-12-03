#pragma once

#ifdef TL_ENABLE_PROFILE

#include "easy/profiler.h"
#define PROFILE_FUNC(...) EASY_FUNCTION(__VA_ARGS__)
#define PROFILE_BLOCK(name, ...) EASY_BLOCK(name, ##__VA_ARGS__)

#else

#define PROFILE_FUNC(...)
#define PROFILE_BLOCK(name, ...)

#endif
