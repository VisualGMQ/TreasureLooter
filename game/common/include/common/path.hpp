#pragma once

#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include <filesystem>

using Path = std::filesystem::path;

// for spdlog output
template <>
struct fmt::formatter<Path> : fmt::ostream_formatter {};