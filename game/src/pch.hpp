#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include "3rdlib/tinyxml2/tinyxml2.h"
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
#include <cstdlib>
#include <string_view>


#define IMGUI_DEFINE_MATH_OPERATORS
#include "3rdlib/imgui/imgui.h"
#include "3rdlib/imgui/imgui_impl_sdl2.h"
#include "3rdlib/imgui/imgui_impl_sdlrenderer2.h"
#include "3rdlib/imgui/imgui_internal.h"
