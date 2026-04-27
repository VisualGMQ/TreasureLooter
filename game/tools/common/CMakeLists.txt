file(GLOB_RECURSE COMMON_SRC ./*.hpp ./*.cpp)

add_library(tool_common STATIC)
target_sources(tool_common PRIVATE ${COMMON_SRC} ${SCHEMA_DISPLAY_OUTPUT_FILES} ${SCHEMA_SERD_OUTPUT_FILES})
target_link_libraries(tool_common PUBLIC ${CLIENT_NAME})
target_include_directories(tool_common PUBLIC .)
if (WIN32)
    target_compile_definitions(tool_common PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()
