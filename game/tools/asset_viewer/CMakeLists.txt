file(GLOB_RECURSE SRC ./*.cpp ./*.hpp)
add_executable(asset_viewer ${SRC} ${SCHEMA_DISPLAY_OUTPUT_FILES} ${SCHEMA_SERD_OUTPUT_FILES})
target_link_libraries(asset_viewer PRIVATE ${ENGINE_NAME} bfg::lyra)
