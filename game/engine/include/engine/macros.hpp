#pragma once
#include "engine/log.hpp"
#include <cassert>

// assert

#ifdef TL_DEBUG 
// Ref:
// https://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros

#define _TL_ASSERT_1_ARGS(x) \
    if (!(x)) {                  \
        LOGC("{}", #x);          \
    }

#define _TL_ASSERT_2_ARGS(x, msg) \
    if (!(x)) {                       \
        LOGC("{}: {}", #x, msg);      \
    }

#define _TL_GET_NTH_ARGS(arg1, arg2, arg3, ...) arg3
#define _TL_ASSERT_CHOOSER(...)                          \
    _TL_GET_NTH_ARGS(__VA_ARGS__, _TL_ASSERT_2_ARGS, \
                         _TL_ASSERT_1_ARGS, )

/**
 * @brief assert macro, has two forms:
 *  1. NICKEL_ASSERT(condition) will assert and log
 *  2. NICKEL_ASSERT(condition, msg) will assert and log additional msg
 */
#define TL_ASSERT(...)                               \
    do {                                                 \
        _TL_ASSERT_CHOOSER(__VA_ARGS__)(__VA_ARGS__) \
    } while (0)

#define TL_CANT_REACH() TL_ASSERT(false, "won't reach here")
#else
#define TL_ASSERT(...)
#define TL_CANT_REACH(msg)
#endif


// logs

#define TL_RETURN_IF_FALSE(expr) \
    do {                             \
        if (!(expr)) return;         \
    } while (0)

#define TL_CONTINUE_IF_FALSE(expr) \
    if (!(expr)) continue;

#define TL_BREAK_IF_FALSE(expr) \
    if (!(expr)) break;

#define TL_RETURN_IF_FALSE_LOGE(expr, msg, ...)     \
    do {                                                \
        if (!(expr)) {                                  \
            LOGE(#expr " failed: " msg, ##__VA_ARGS__); \
            return;                                     \
        }                                               \
    } while (0)

#define TL_RETURN_IF_FALSE_LOGI(expr, msg, ...)     \
    do {                                                \
        if (!(expr)) {                                  \
            LOGI(#expr " failed: " msg, ##__VA_ARGS__); \
            return;                                     \
        }                                               \
    } while (0)

#define TL_RETURN_IF_FALSE_LOGW(expr, msg, ...)     \
    do {                                                \
        if (!(expr)) {                                  \
            LOGW(#expr " failed: " msg, ##__VA_ARGS__); \
            return;                                     \
        }                                               \
    } while (0)

#define TL_RETURN_VALUE_IF_FALSE_LOGW(value, expr, msg, ...) \
    do {                                                         \
        if (!(expr)) {                                           \
            LOGW(#expr " failed: " msg, ##__VA_ARGS__);          \
            return value;                                        \
        }                                                        \
    } while (0)

#define TL_RETURN_VALUE_IF_FALSE_LOGE(value, expr, msg, ...) \
    do {                                                         \
        if (!(expr)) {                                           \
            LOGE(#expr " failed: " msg, ##__VA_ARGS__);          \
            return value;                                        \
        }                                                        \
    } while (0)