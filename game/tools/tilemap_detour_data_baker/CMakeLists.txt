file(GLOB_RECURSE SRC ./*.cpp ./*.hpp)
add_executable(tilemap_detour_data_builder ${SRC})
target_link_libraries(tilemap_detour_data_builder PRIVATE tool_common bfg::lyra)
