#include "context.hpp"
#include "flags.hpp"
#include "log.hpp"
#include "math.hpp"
#include "renderer.hpp"
#include "transform.hpp"

namespace tl {

Context* Context::inst = nullptr;

void Context::Init() {
    inst = new Context;
}

void Context::Destroy() {
    delete inst;
}

Context::Context() : window{"Treasure Looter", 1024, 720}, renderer{window} {
    if (!window || !renderer) {
        quitSDL();
        exit(1);
    }
}

void Context::initSDL() {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    Mix_Init(MIX_INIT_MP3);
    TTF_Init();
}

void Context::quitSDL() {
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Context::Update() {
    while (!shouldExit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldExit = true;
            }
        }

        renderer.Clear(Color{100, 100, 100});
        updateGO(nullptr, goMgr.GetRootGO());
        debugMgr.Update();
        renderer.Present();
    }
}

void Context::updateGO(GameObject* parent, GameObject* go) {
    if (!parent) {
        go->globalTransform_ = go->transform;
    } else {
        go->globalTransform_ =
            CalcTransformFromParent(parent->globalTransform_, go->transform);
    }

    drawSprite(*go);

    for (GameObjectID id : go->children) {
        GameObject* child = goMgr.Find(id);
        if (child) {
            updateGO(go, child);
        }
    }
}

void Context::drawSprite(GameObject& go) {
    if (!go.sprite.isEnable || !go.sprite) {
        return;
    }

    const Transform& goTrans = go.GetGlobalTransform();

    Transform globalTransform =
        CalcTransformFromParent(goTrans, go.sprite.transform);
    Vec2 xAxis = Rotate(Vec2::X_AXIS, goTrans.rotation);
    Vec2 yAxis = Rotate(Vec2::Y_AXIS, goTrans.rotation);
    Rect dstRect;
    Vec2 unsignedScale = globalTransform.scale;
    unsignedScale.x = std::abs(globalTransform.scale.x);
    unsignedScale.y = std::abs(globalTransform.scale.y);
    dstRect.size = unsignedScale * go.sprite.GetRegion().size;
    dstRect.position = globalTransform.position -
                       dstRect.size.w * go.sprite.GetAnchor().x * xAxis -
                       dstRect.size.h * go.sprite.GetAnchor().y * yAxis;

    Flags<Flip> flip = Flip::None;
    if (globalTransform.scale.x < 0) {
        flip |= Flip::Horizontal;
    }
    if (globalTransform.scale.y < 0) {
        flip |= Flip::Vertical;
    }

    renderer.DrawTexture(*go.sprite.GetTexture(), go.sprite.GetRegion(),
                         dstRect, globalTransform.rotation,
                         go.sprite.GetAnchor() * go.sprite.GetRegion().size *
                             globalTransform.scale,
                         flip);
}

}  // namespace tl
