#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>


#define IMGUI_DEFINE_MATH_OPERATORS
#include "3rdlib/imgui/imgui.h"
#include "3rdlib/imgui/imgui_impl_sdl2.h"
#include "3rdlib/imgui/imgui_impl_sdlrenderer2.h"
#include "3rdlib/imgui/imgui_internal.h"


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