#pragma once

#include "schema/prefab.hpp"

class PrefabManager : public AssetManagerBase<Prefab> {
public:
    HandleType Load(const Path& filename, bool force = false) override;
    HandleType Create();
    HandleType Create(const Prefab& value, const Path& filename);

private:
    void changePrefabByBase(Prefab& prefab);
};
