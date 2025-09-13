#pragma once

#include "SDL3/SDL.h"
#include "math.hpp"
#include <string>

class Window {
public:
    Window(const std::string& title, int w, int h);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    ~Window();

    Vec2 GetWindowSize() const;
    void SetTitle(const std::string&);
    void Resize(const Vec2&);
    SDL_WindowID GetID() const;

    SDL_Window* GetWindow();
    
private:
    SDL_Window* m_window{};
};