#include "engine/log.hpp"

#include "SDL3/SDL.h"
#include "engine/sdl_call.hpp"
#include "spdlog/sinks/android_sink.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <iostream>

LogManager LogManager::manager;

LogManager& LogManager::GetInst() {
    return manager;
}

LogManager::LogManager() {
#ifdef SDL_PLATFORM_ANDROID
    std::string tag = "spdlog-android";
    m_console_logger = spdlog::android_logger_mt("android", tag);

    m_file_logger = spdlog::null_logger_mt("file_logger");
#else
    m_console_logger = spdlog::stdout_color_mt("console");

    try {
        SDL_Time time;
        SDL_CALL(SDL_GetCurrentTime(&time));
        SDL_DateTime date;
        SDL_CALL(SDL_TimeToDateTime(time, &date, true));

        std::string filename =
            std::to_string(date.year) + "_" + std::to_string(date.month) + "_" +
            std::to_string(date.day) + "-" + std::to_string(date.hour) + "_" +
            std::to_string(date.minute) + "_" + std::to_string(date.second) +
            ".log";
        m_file_logger = spdlog::basic_logger_mt("file_logger", "logs/" + filename);
    } catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
#endif
}