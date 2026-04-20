#include "server/context.hpp"
#include "SDL3_ttf/SDL_ttf.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/cct.hpp"
#include "common/context.hpp"
#include "common/debug_drawer.hpp"
#include "common/event.hpp"
#include "common/log.hpp"
#include "common/profile.hpp"
#include "common/relationship.hpp"
#include "common/scene.hpp"
#include "common/script/script.hpp"
#include "common/sdl_call.hpp"
#include "common/serialize.hpp"
#include "common/static_collision.hpp"
#include "common/storage.hpp"
#include "common/tilemap.hpp"
#include "common/transform.hpp"
#include "common/trigger.hpp"
#include "common/uuid.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "schema/asset_info.hpp"
#include "schema/config.hpp"
#include "schema/serialize/input.hpp"
#include "schema/serialize/prefab.hpp"
#include "server/asset_manager.hpp"
#include "server/scene.hpp"

#include <memory>

void ServerContext::Initialize(int argc, char** argv) {
    PROFILE_SECTION();

    CommonContext::Initialize(argc, argv);
    m_assets_manager = std::make_unique<ServerAssetsManager>();
    m_scene_manager = std::unique_ptr<ServerSceneManager>(new ServerSceneManager{});

    CommonContext::initGameConfig();

    m_assets_manager->GetManager<ScriptBinaryData>().Initialize(GetGameConfig());

    m_debug_drawer = std::unique_ptr<IDebugDrawer>(new TrivialDebugDrawer{});

    m_time->SetFPS(60);
}

void ServerContext::AttachComponentsOnEntity(Entity, const EntityInstance&) {
    // TODO: not finish
}
