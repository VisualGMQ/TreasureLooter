#pragma once
#include "animation.hpp"
#include "schema/input.hpp"
#include "schema/prefab.hpp"

#include <variant>

using AssetTypes =
    std::variant<std::monostate, AssetLoadResult<Prefab>,
                 AssetLoadResult<InputConfig>, AssetLoadResult<Animation>>;

class Editor {
public:
    void Update();

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