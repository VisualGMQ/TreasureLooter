#pragma once

#include "log.hpp"
#include "pch.hpp"

#define TL_RETURN_VALUE_IF_FALSE(x, value) \
    do {                                   \
        if (!(x)) return value;            \
    } while (0)

#define TL_RETURN_TRUE_IF_FALSE(x) \
    do {                           \
        if (!(x)) return true;     \
    } while (0)

#define TL_RETURN_FALSE_IF_FALSE(x) \
    do {                            \
        if (!(x)) return false;     \
    } while (0)

#define TL_RETURN_IF_FALSE(x) \
    do {                      \
        if (!(x)) return;     \
    } while (0)

#define TL_RETURN_NULL_IF_FALSE(x) \
    do {                           \
        if (!(x)) return nullptr;  \
    } while (0)

#define TL_RETURN_IF_FALSE_LOGE(x, msg, ...) \
    do {                                     \
        if (!(x)) {                          \
            LOGE(msg, ##__VA_ARGS__);        \
            return;                          \
        }                                    \
    } while (0)

#define TL_RETURN_NULL_IF_FALSE_LOGE(x, msg, ...) \
    do {                                          \
        if (!(x)) {                               \
            LOGE(msg, ##__VA_ARGS__);             \
            return nullptr;                       \
        }                                         \
    } while (0)

#define TL_RETURN_IF_FALSE_LOGW(x, msg, ...) \
    do {                                     \
        if (!(x)) {                          \
            LOGW(msg, ##__VA_ARGS__);        \
            return;                          \
        }                                    \
    } while (0)

#define TL_RETURN_NULL_IF_FALSE_LOGW(x, msg, ...) \
    do {                                          \
        if (!(x)) {                               \
            LOGW(msg, ##__VA_ARGS__);             \
            return nullptr;                       \
        }                                         \
    } while (0)

#define TL_RETURN_DEFAULT_IF_FALSE_LOGW(x, msg, ...) \
    do {                                             \
        if (!(x)) {                                  \
            LOGW(msg, ##__VA_ARGS__);                \
            return {};                               \
        }                                            \
    } while (0)

#define TL_CONTINUE_IF_FALSE(x) \
    if (!(x)) {                 \
        continue;               \
    }

#define TL_CONTINUE_IF_FALSE_WITH_LOGW(x, msg, ...) \
    if (!(x)) {                                     \
        LOGW(msg, ##__VA_ARGS__);                   \
        continue;                                   \
    }


#define TL_CONTINUE_IF(x) \
    if ((x)) {            \
        continue;         \
    }

#define TL_BREAK_IF_FALSE(x) \
    if (!(x)) {              \
        break;               \
    }

#define FLT_EQ(a, b) (std::abs(a - b) <= std::numeric_limits<float>::epsilon())