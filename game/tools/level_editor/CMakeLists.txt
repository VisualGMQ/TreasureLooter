file(GLOB_RECURSE SRC ./*.cpp ./*.hpp)
add_executable(level_editor ${SRC})
target_link_libraries(level_editor PRIVATE ${ENGINE_NAME})
