#pragma once
#include <assert.h>

#define TL_RETURN_IF_NULL(expr)        \
    do {                               \
        if ((expr) == nullptr) return; \
    } while (0)

#define TL_RETURN_IF_FALSE(expr) \
    do {                         \
        if (!(expr)) return;     \
    } while (0)

#define TL_RETURN_IF_TRUE(expr) \
    do {                        \
        if (expr) return;       \
    } while (0)

#define TL_CONTINUE_IF_FALSE(expr) \
    if (!(expr)) {                 \
        continue;                  \
    }

#define TL_CONTINUE_IF_NULL(expr) \
    if ((expr) == nullptr) {      \
        continue;                  \
    }

#define TL_CONTINUE_IF_TRUE(expr) \
    if (expr) {                   \
        continue;                 \
    }

#define TL_BREAK_IF_FALSE(expr) \
    if (!(expr)) {              \
        break;                  \
    }

#define TL_BREAK_IF_TRUE(expr) \
    if (expr) {                \
        break;                 \
    }

#define TL_RETURN_DEFAULT_IF_NULL(expr)   \
    do {                                  \
        if ((expr) == nullptr) return {}; \
    } while (0)

#define TL_RETURN_DEFAULT_IF_FALSE(expr) \
    do {                                 \
        if (!(expr)) return {};          \
    } while (0)

#define TL_RETURN_DEFAULT_IF_TRUE(expr) \
    do {                                \
        if (expr) return {};            \
    } while (0)

#define TL_RETURN_IF_NULL_WITH_LOG(expr, log_fn, fmt, ...) \
    do {                                                   \
        if ((expr) == nullptr) {                           \
            log_fn(fmt, ##__VA_ARGS__);                    \
            return;                                        \
        }                                                  \
    } while (0)

#define TL_RETURN_IF_FALSE_WITH_LOG(expr, log_fn, fmt, ...) \
    do {                                                    \
        if (!(expr)) {                                      \
            log_fn(fmt, ##__VA_ARGS__);                     \
            return;                                         \
        }                                                   \
    } while (0)

#define TL_RETURN_IF_TRUE_WITH_LOG(expr, log_fn, fmt, ...) \
    do {                                                   \
        if (expr) {                                        \
            log_fn(fmt, ##__VA_ARGS__);                    \
            return;                                        \
        }                                                  \
    } while (0)

#define TL_RETURN_IF_FALSE(expr) \
    do {                         \
        if (!(expr)) return;     \
    } while (0)

#define TL_RETURN_IF_TRUE(expr) \
    do {                        \
        if (expr) return;       \
    } while (0)

#define TL_RETURN_DEFAULT_IF_NULL_WITH_LOG(expr, log_fn, fmt, ...) \
    do {                                                           \
        if ((expr) == nullptr) {                                   \
            log_fn(fmt, ##__VA_ARGS__);                            \
            return {};                                             \
        }                                                          \
    } while (0)

#define TL_RETURN_DEFAULT_IF_FALSE_WITH_LOG(expr, log_fn, fmt, ...) \
    do {                                                            \
        if (!(expr)) {                                              \
            log_fn(fmt, ##__VA_ARGS__);                             \
            return {};                                              \
        }                                                           \
    } while (0)

#define TL_RETURN_DEFAULT_IF_TRUE_WITH_LOG(expr, log_fn, fmt, ...) \
    do {                                                           \
        if (!(expr)) {                                             \
            log_fn(fmt, ##__VA_ARGS__);                            \
            return {};                                             \
        }                                                          \
    } while (0)

#define TL_ASSERT(expr) assert(expr)