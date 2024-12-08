#pragma once

namespace tl {

class Scene;

class Level {
public:
    friend class SceneManager;

    Level(Scene& scene): scene_{scene} {}
    Level(const Level&) = delete;
    Level& operator=(const Level&) = delete;
    virtual ~Level() = default;

    virtual void Init() {}

    virtual void Enter() {}

    virtual void Quit() {}

    virtual void Update() {}

    bool IsInited() const { return isInited_; }
    const Scene& GetScene() const { return scene_; }
    Scene& GetScene() { return scene_; }

private:
    bool isInited_ = false;
    Scene& scene_;
};

}  // namespace tl