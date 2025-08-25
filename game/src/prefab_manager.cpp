#include "prefab_manager.hpp"
#include "schema/serialize/prefab.hpp"

AssetManagerBase<Prefab>::HandleType PrefabManager::Load(const Path& filename,
    bool force) {
    auto result = LoadAsset<Prefab>(filename);
    changePrefabByBase(result.m_payload);
    return this->store(&filename, result.m_uuid,
                       std::make_unique<Prefab>(std::move(result.m_payload)));
}

AssetManagerBase<Prefab>::HandleType PrefabManager::Create() {
    return this->store(nullptr, UUID::CreateV4(), std::make_unique<Prefab>());
}

AssetManagerBase<Prefab>::HandleType PrefabManager::
Create(const Prefab& value) {
    return this->store(nullptr, UUID::CreateV4(),
                       std::make_unique<Prefab>(value));
}

#define REPLACE_BY_BASE(property) if (base->property && !prefab.property) { prefab.property = base->property; }

void PrefabManager::changePrefabByBase(Prefab& prefab) {
    if (!prefab.m_base) {
        return;
    }

    auto base = prefab.m_base;
    REPLACE_BY_BASE(m_transform)
    REPLACE_BY_BASE(m_sprite)
    REPLACE_BY_BASE(m_animation)
    REPLACE_BY_BASE(m_tilemap)
    REPLACE_BY_BASE(m_cct)
    REPLACE_BY_BASE(m_trigger)
    REPLACE_BY_BASE(m_motor_config)

    if (!prefab.m_base->m_children.empty() && prefab.m_children.empty()) {
        prefab.m_children = prefab.m_base->m_children;
    }
}
