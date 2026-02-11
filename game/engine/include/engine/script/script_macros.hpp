#pragma once
#include "angelscript.h"
#include "engine/macros.hpp"
#include "engine/log.hpp"

#define AS_CALL(expr)                                                       \
    do {                                                                    \
        if ((expr) < 0) LOGE("[AngelScript]: execute `{}`, failed", #expr); \
    } while (0)

#define AS_CALL_WITH_MSG(expr, msg)                                      \
    do {                                                                 \
        if ((expr) < 0)                                                  \
            LOGE("[AngelScript]: execute `{}`, failed: {}", #expr, msg); \
    } while (0)

#define AS_CALL_WITH_RETURN(expr)                               \
    do {                                                        \
        if ((expr) < 0) {                                       \
            LOGE("[AngelScript]: execute `{}`, failed", #expr); \
            return;                                             \
        }                                                       \
    } while (0)

#define AS_CALL_WITH_RETURN_AND_MSG(expr, msg)                           \
    do {                                                                 \
        if ((expr) < 0) {                                                \
            LOGE("[AngelScript]: execute `{}`, failed: {}", #expr, msg); \
            return;                                                      \
        }                                                                \
    } while (0)


