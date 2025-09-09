#pragma once

#include "SDL3/SDL.h"
#include "log.hpp"

#define SDL_CALL(expr)                             \
    do {                                           \
        if (!(expr)) {                             \
            LOGE("SDL Error: {}", SDL_GetError()); \
        }                                          \
    } while (0)