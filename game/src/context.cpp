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

Context::Context() {
    initSDL();

    window = std::make_unique<Window>("Treasure Looter", 1024, 720);
    renderer = std::make_unique<Renderer>(*window);
    if (!window || !renderer) {
        quitSDL();
        exit(1);
    }

    textureMgr = std::make_unique<TextureManager>();
    goMgr = std::make_unique<GameObjectManager>();
    animMgr = std::make_unique<AnimationManager>();
    debugMgr = std::make_unique<DebugManager>();
}

void Context::initSDL() {
    SDL_Init(SDL_INIT_EVERYTHING);
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
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

Context::~Context() {
    debugMgr.reset();
    animMgr.reset();
    goMgr.reset();
    textureMgr.reset();
    renderer.reset();
    window.reset();

    quitSDL();
}

void Context::Update() {
    while (!shouldExit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldExit = true;
            }
        }

        renderer->Clear(Color{100, 100, 100});
        updateGO(nullptr, goMgr->GetRootGO());
        debugMgr->Update();
        renderer->Present();
    }
}

void Context::updateGO(GameObject* parent, GameObject* go) {
    if (go->animator.animation) {
        go->animator.animation->Update(1);
    }
    syncAnim2GO(*go);
    if (!parent) {
        go->globalTransform_ = go->transform;
    } else {
        go->globalTransform_ =
            CalcTransformFromParent(parent->globalTransform_, go->transform);
    }
    drawSprite(*go);

    for (GameObjectID id : go->children) {
        GameObject* child = goMgr->Find(id);
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

    renderer->DrawTexture(*go.sprite.GetTexture(), go.sprite.GetRegion(),
                         dstRect, globalTransform.rotation,
                         go.sprite.GetAnchor() * go.sprite.GetRegion().size *
                             globalTransform.scale,
                         flip);
}

void Context::syncAnim2GO(GameObject& go) {
    if (!go.animator) {
        return;
    }

    auto& floatTrack = go.animator.animation->GetFloatTracks();
    for (auto& [bind, track] : floatTrack) {
        switch (bind) {
            case FloatBindPoint::GORotation:
                go.transform.rotation = track.curData;
                break;
            case FloatBindPoint::SpriteRotation:
                go.sprite.transform.rotation = track.curData;
                break;
        }
    }

    auto& vec2Track = go.animator.animation->GetVec2Tracks();
    for (auto& [bind, track] : vec2Track) {
        switch (bind) {
            case Vec2BindPoint::GOPosition:
                go.transform.position = track.curData;
                break;
            case Vec2BindPoint::GOScale:
                go.transform.scale = track.curData;
                break;
            case Vec2BindPoint::SpritePosition:
                go.sprite.transform.position = track.curData;
                break;
            case Vec2BindPoint::SpriteScale:
                go.sprite.transform.scale = track.curData;
                break;
        }
    }

    auto& textureTrack = go.animator.animation->GetTextureTracks();
    for (auto& [bind, track] : textureTrack) {
        switch (bind) {
            case TextureBindPoint::Sprite:
                go.sprite.SetTexture(*track.curData);
                break;
        }
    }

    auto& rectTrack = go.animator.animation->GetRectTracks();
    for (auto& [bind, track] : rectTrack) {
        switch (bind) {
            case RectBindPoint::Sprite:
                go.sprite.SetRegion(track.curData);
                break;
        }
    }
}

}  // namespace tl
