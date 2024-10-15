#pragma once

namespace tl {

class Level {
public:
    friend class SceneManager;

    virtual ~Level() = default;
    virtual void Init() = 0;
    virtual void Enter() = 0;
    virtual void Quit() = 0;
    virtual void Update() = 0;
    bool IsInited() const { return isInited_; }

private:
    bool isInited_ = false;
};

}