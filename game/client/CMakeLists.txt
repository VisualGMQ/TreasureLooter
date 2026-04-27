file(GLOB_RECURSE CLIENT_SRC CONFIGURE_DEPENDS src/*.cpp src/*.c)
file(GLOB_RECURSE CLIENT_HEADER CONFIGURE_DEPENDS include/*.hpp include/*.h)

add_library(${CLIENT_NAME} STATIC)

target_sources(${CLIENT_NAME} PRIVATE ${CLIENT_SRC} ${CLIENT_HEADER} ${ASSETS})
target_include_directories(${CLIENT_NAME} PUBLIC include ../schema_generate)

# TODO: compatible with clang++
if (MSVC)
    target_compile_options(${CLIENT_NAME} PRIVATE /bigobj)
endif()

target_compile_definitions(${CLIENT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(${CLIENT_NAME} PUBLIC ${COMMON_NAME})
