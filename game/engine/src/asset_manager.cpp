#include "engine/asset_manager.hpp"

AssetsManager::AssetsManager(){
    m_script_binary_data_manager = std::make_unique<ScriptBinaryDataManager>();
}

AssetsManager::~AssetsManager() {
    m_managers.clear();
    m_script_binary_data_manager.reset();
}