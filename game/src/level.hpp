#pragma once

namespace tl {

class Level {
public:
    friend class SceneManager;

    virtual ~Level() = default;
    virtual void Init() {}
    virtual void Enter() {}
    virtual void Quit() {}
    virtual void Update() {}
    bool IsInited() const { return isInited_; }

private:
    bool isInited_ = false;
};

}