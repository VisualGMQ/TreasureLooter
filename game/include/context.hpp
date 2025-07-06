#pragma once
#include "gameobject.hpp"
#include "image.hpp"
#include "inspector.hpp"
#include "renderer.hpp"
#include "window.hpp"
#include <memory>

class Context {
public:
    static void Init();
    static void Destroy();
    static Context& GetInst();

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;
    ~Context();

    void Update();
    void HandleEvents(const SDL_Event&);
    bool ShouldExit();

private:
    static std::unique_ptr<Context> instance;
    
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<ImageManager> m_image_manager;
    std::unique_ptr<Inspector> m_inspector;
    bool m_should_exit = false;

    GameObject m_root;

    Context();

    void drawSpriteRecursive(const GameObject&);
    void updateGOPoses();
    void updatePoseRecursive(const GameObject& parent, GameObject& child);

    void logicUpdate();
    void renderUpdate();
};