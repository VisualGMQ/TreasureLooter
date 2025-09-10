#pragma once
#include "common.hpp"

class AssetViewer {
public:
    AssetViewer();

    void LoadAsset(const Path& filename);
    void Update();

private:
    bool m_is_open = true;
    Path m_current_open_filename;
    VariantAsset m_asset;
    int m_id{};
    std::string window_id;
    Vec2 m_window_size{720, 640};

    void changeID(const Path& filename);

    void showMenu();
};
