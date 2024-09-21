#pragma once
#include "pch.hpp"

namespace tl {

#define LOG(level, fmt, ...)                                              \
    do {                                                                  \
        printf("[%s]%s|%s[%d]: " fmt "\n", level, __FILE__, __FUNCTION__, \
               __LINE__, ##__VA_ARGS__);                                  \
    } while (0)

#define LOGI(fmt, ...) LOG("INFO", fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) LOG("WARN", fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOG("ERROR", fmt, ##__VA_ARGS__)

}  // namespace tl
