file(GLOB_RECURSE SERVER_SRC CONFIGURE_DEPENDS src/*.cpp src/*.c)
file(GLOB_RECURSE SERVER_HEADER CONFIGURE_DEPENDS include/*.hpp include/*.h)

add_library(${SERVER_NAME} STATIC)

target_sources(${SERVER_NAME} PRIVATE ${SERVER_SRC} ${SERVER_HEADER} ${ASSETS})
target_include_directories(${SERVER_NAME} PUBLIC include ../schema_generate)

# TODO: compatible with clang++
if (MSVC)
    target_compile_options(${SERVER_NAME} PRIVATE /bigobj)
endif()

target_compile_definitions(${SERVER_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(${SERVER_NAME} PUBLIC ${COMMON_NAME})
