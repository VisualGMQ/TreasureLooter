#include "window.hpp"
#include "log.hpp"

namespace tl {

Window::Window(const std::string& title, int w, int h) {
#ifdef TL_ANDROID
    window = SDL_CreateWindow(title.c_str(), 0, 0, 0, 0, SDL_WINDOW_SHOWN);
#else
    window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
#endif

    if (!window_) {
        LOGE("window create failed");
    }
}

Window::Window(Window&& o) : window_{o.window_} {
    o.window_ = nullptr;
}

Window& Window::operator=(Window&& o) {
    if (&o != this) {
        window_ = o.window_;
        o.window_ = nullptr;
    }
    return *this;
}

Vec2 Window::GetSize() const {
    int w, h;
    SDL_GetWindowSize(window_, &w, &h);
    return Vec2(w, h);
}

Window::operator bool() const {
    return window_;
}

Window::~Window() {
    SDL_DestroyWindow(window_);
}

}  // namespace tl
