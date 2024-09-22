#pragma once
#include "pch.hpp"
#include "math.hpp"

namespace tl {

class Window {
public:
    friend class Renderer;

    Window(const std::string& title, int w, int h);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&);
    Window& operator=(Window&&);
    ~Window();

    Vec2 GetSize() const;

    operator bool() const;

private:
    SDL_Window* window_;
};

}
