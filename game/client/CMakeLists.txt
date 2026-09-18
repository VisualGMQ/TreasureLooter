file(GLOB_RECURSE CLIENT_SRC CONFIGURE_DEPENDS src/*.cpp src/*.c)
file(GLOB_RECURSE CLIENT_HEADER CONFIGURE_DEPENDS include/*.hpp include/*.h)

add_library(${CLIENT_MODULE_NAME} STATIC)

target_sources(${CLIENT_MODULE_NAME} PRIVATE ${CLIENT_SRC} ${CLIENT_HEADER} ${ASSETS})
target_include_directories(${CLIENT_MODULE_NAME} PUBLIC include)

target_compile_definitions(${CLIENT_MODULE_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(${CLIENT_MODULE_NAME}
    PUBLIC
    ${COMMON_MODULE_NAME}
    fts_fuzzy_match
    stb_image
    imgui
    ImNodeFlow)
