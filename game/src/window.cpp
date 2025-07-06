#include "window.hpp"

#include "log.hpp"
#include "sdl_call.hpp"

Window::Window(const std::string& title, int w, int h) {
#ifdef SDL_PLATFORM_ANDROID
    m_window = SDL_CreateWindow(title.c_str(), 0, 0, 0);
#else
    m_window = SDL_CreateWindow(title.c_str(), w, h, SDL_WINDOW_RESIZABLE);
#endif
    if (!m_window) {
        LOGE("create SDL window failed! {}", SDL_GetError());
    }
}

Window::~Window() {
    SDL_DestroyWindow(m_window);
}

Vec2 Window::GetWindowSize() const {
    int w, h;
    SDL_CALL(SDL_GetWindowSize(m_window, &w, &h));
    return Vec2(w, h);
}

SDL_Window* Window::GetWindow() {
    return m_window;
}