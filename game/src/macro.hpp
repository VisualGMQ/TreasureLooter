#pragma once

#include "pch.hpp"
#include "log.hpp"

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

#define TL_RETURN_NULL_IF(x)      \
    do {                          \
        if (!(x)) return nullptr; \
    } while (0)

#define TL_RETURN_IF_LOGE(x, msg, ...) \
    do {                               \
        if (!(x)) {                    \
            LOGE(msg, ##__VA_ARGS__);  \
            return;                    \
        }                              \
    } while (0)

#define TL_RETURN_NULL_IF_LOGE(x, msg, ...) \
    do {                                    \
        if (!(x)) {                         \
            LOGE(msg, ##__VA_ARGS__);       \
            return nullptr;                 \
        }                                   \
    } while (0)

#define TL_RETURN_IF_LOGW(x, msg, ...) \
    do {                               \
        if (!(x)) {                    \
            LOGW(msg, ##__VA_ARGS__);  \
            return;                    \
        }                              \
    } while (0)

#define TL_RETURN_NULL_IF_LOGW(x, msg, ...) \
    do {                                    \
        if (!(x)) {                         \
            LOGW(msg, ##__VA_ARGS__);       \
            return nullptr;                 \
        }                                   \
    } while (0)

#define TL_CONTINUE_IF(x) \
    if (!(x)) {           \
        continue;         \
    }

#define TL_BREAK_IF(x) \
    if (!(x)) {        \
        break;         \
    }

#define FLT_EQ(a, b) (std::abs(a - b) <= std::numeric_limits<float>::epsilon())