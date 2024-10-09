#include "pch.hpp"
#include "context.hpp"

#ifdef __EMSCRIPTEN__
void MainLoop() {
    tl::Context::GetInst().Update();
}
#endif

int main(int argc, char** argv) {
    tl::Context::Init();
    tl::Context& ctx = tl::Context::GetInst();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(MainLoop, 0, true);
#else
    while (!ctx.ShouldExit()) {
        ctx.Update();
    }
#endif
    tl::Context::Destroy();

    return 0;
}
