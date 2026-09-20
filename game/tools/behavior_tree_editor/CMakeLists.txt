file(GLOB_RECURSE SRC ./*.cpp ./*.hpp)
add_executable(behavior_tree_editor ${SRC})
target_link_libraries(behavior_tree_editor PRIVATE tool_common bfg::lyra)
