#include "engine/window.hpp"

#include "engine/log.hpp"
#include "engine/sdl_call.hpp"

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

void Window::SetTitle(const std::string& title) {
    SDL_CALL(SDL_SetWindowTitle(m_window, title.c_str()));
}

SDL_WindowID Window::GetID() const {
    return SDL_GetWindowID(m_window);
}

SDL_Window* Window::GetWindow() {
    return m_window;
}

void Window::Resize(const Vec2UI& size) {
    SDL_CALL(SDL_SetWindowSize(m_window, size.x, size.y));
}
