file(GLOB_RECURSE SRC ./*.cpp ./*.hpp)
add_executable(collision_editor ${SRC})
target_link_libraries(collision_editor PRIVATE tool_common bfg::lyra)
