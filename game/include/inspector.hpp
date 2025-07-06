#pragma once
#include <SDL3/SDL_events.h>

class Window;
class Renderer;

class Inspector {
public:
    Inspector(Window& window, Renderer& renderer);
    ~Inspector();

    void BeginFrame();
    void EndFrame();
    void Update();

    void HandleEvents(const SDL_Event& event);
    
private:
    Window& m_window;
    Renderer& m_renderer;
};