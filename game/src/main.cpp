#include "context.hpp"
#include "pch.hpp"
#include "profile.hpp"
#include <ctime>
#include <filesystem>

#ifdef __EMSCRIPTEN__
void MainLoop() {
    tl::Context::GetInst().Update();
}
#endif

int main(int argc, char** argv) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    tm* tm = std::localtime(&now_time);
    PROFILE_STARTUP();

#ifdef TL_ENABLE_PROFILE
    // profiler
    auto curPath = std::filesystem::current_path();
    curPath /= "log";
    if (!std::filesystem::exists(curPath)) {
        std::filesystem::create_directory(curPath);
    }
#endif

    tl::Context::Init();
    tl::Context& ctx = tl::Context::GetInst();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(MainLoop, 0, true);
#else

    while (!ctx.ShouldExit()) {
        PROFILE_FRAME("MainLoop");
        ctx.Update();
    }
#endif
    tl::Context::Destroy();

#ifdef TL_ENABLE_PROFILE
    char filename[1024] = {0};
    snprintf(filename, sizeof(filename), "log/%d-%d-%d_%d-%d-%d.prof", tm->tm_year,
             tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    // TODO: save profile file 
    PROFILE_SHUTDOWN();
#endif

    return 0;
}
