file(GLOB_RECURSE COMMON_SRC ./*.hpp ./*.cpp)

add_library(tool_common STATIC)
target_sources(tool_common PRIVATE ${COMMON_SRC} ${SCHEMA_DISPLAY_OUTPUT_FILES} ${SCHEMA_SERD_OUTPUT_FILES})
target_link_libraries(tool_common PUBLIC ${ENGINE_NAME})
target_include_directories(tool_common PUBLIC .)
