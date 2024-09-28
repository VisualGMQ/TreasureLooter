#pragma once

#include <cstdio>
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <cassert>
#include <numeric>
#include <limits>
#include <memory>
#include <vector>
#include <string>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "3rdlib/imgui/imgui.h"
#include "3rdlib/imgui/imgui_internal.h"
#include "3rdlib/imgui/imgui_impl_sdl2.h"
#include "3rdlib/imgui/imgui_impl_sdlrenderer2.h"

#define TL_RETURN_VALUE_IF(x, value) \
    do {                             \
        if (!(x)) return value;      \
    } while (0)
#define TL_RETURN_TRUE_IF(x)   \
    do {                       \
        if (!(x)) return true; \
    } while (0)
#define TL_RETURN_FALSE_IF(x)   \
    do {                        \
        if (!(x)) return false; \
    } while (0)
#define TL_RETURN_IF(x)   \
    do {                  \
        if (!(x)) return; \
    } while (0)