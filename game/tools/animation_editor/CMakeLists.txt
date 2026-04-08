file(GLOB_RECURSE SRC ./*.cpp ./*.hpp)
add_executable(animation_editor ${SRC})
target_link_libraries(animation_editor PRIVATE tool_common bfg::lyra)
