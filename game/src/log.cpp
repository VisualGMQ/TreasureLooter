#include "log.hpp"
#include "SDL3/SDL.h"
#include "spdlog/sinks/android_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <iostream>


LogManager LogManager::manager;

LogManager& LogManager::GetInst() {
    return manager;
}

LogManager::LogManager() {
#ifdef SDL_PLATFORM_ANDROID
    std::string tag = "spdlog-android";
    m_console_logger = spdlog::android_logger_mt("android", tag);
#else
    m_console_logger = spdlog::stdout_color_mt("console");

    try {
        m_file_logger = spdlog::basic_logger_mt("file_logger", "logs/log.txt");
    } catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
#endif
}