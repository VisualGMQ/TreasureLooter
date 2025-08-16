#pragma once
#include "animation.hpp"
#include "schema/config.hpp"
#include "schema/input.hpp"
#include "schema/prefab.hpp"

#include <variant>

class IEditor {
public:
    virtual ~IEditor() = default;
    virtual void Update() = 0;
};

class TrivialEditor : public IEditor {
    void Update() override {};
};

#ifdef TL_ENABLE_EDITOR

using AssetTypes = std::variant < std::monostate, AssetLoadResult<Prefab>,
      AssetLoadResult<InputConfig>, AssetLoadResult<Animation>,
      AssetLoadResult<GameConfig>, AssetLoadResult<LevelContent>,
      AssetLoadResult<PhysicsActorInfo>>;

class Editor : public IEditor {
public:
    void Update() override;

private:
    enum class Mode {
        None,
        Open,
        Create,
    };

    Mode m_mode = Mode::None;

    AssetTypes m_asset;
    Path m_filename;
    std::optional<size_t> m_asset_index;

    bool m_open = true;
};

#endif