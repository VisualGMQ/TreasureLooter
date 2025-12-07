file(GLOB_RECURSE SRC ./*.cpp ./*.hpp)
add_executable(asset_editor ${SRC})
target_link_libraries(asset_editor PRIVATE tool_common bfg::lyra)
