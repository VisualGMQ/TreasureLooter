#pragma once

#ifdef TL_ENABLE_PROFILE
#include "tracy/Tracy.hpp"

#define PROFILE_FRAME() FrameMark
#define PROFILE_FRAME_NAMED(name) FrameMarkNamed(name)
#define PROFILE_FRAME_START(name) FrameMarkStart(name)
#define PROFILE_FRAME_END(name) FrameMarkEnd(name)

#define PROFILE_SECTION() ZoneScoped
#define PROFILE_SECION_NAMED(name) ZoneScopedN(name)
#define PROFILE_SECTION_NAMED_COLORED(name, color) ZoneScopedNC(name, color)

#else

#define PROFILE_FRAME()
#define PROFILE_FRAME_NAMED(name)
#define PROFILE_FRAME_START(name)
#define PROFILE_FRAME_END(name)

#define PROFILE_SECTION()
#define PROFILE_SECION_NAMED(name)
#define PROFILE_SECTION_NAMED_COLORED(name, color)

#endif
