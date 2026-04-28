# CONFIGURE_DEPENDS: re-run CMake when new .cpp appear (plain GLOB misses new files until configure).
file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS src/*.cpp src/*.c)
file(GLOB_RECURSE ENGINE_HEADER CONFIGURE_DEPENDS include/*.hpp include/*.h)
file(GLOB_RECURSE ASSETS ../assets/*)

add_library(${COMMON_NAME} STATIC)

target_sources(${COMMON_NAME} PRIVATE ${ENGINE_SRC} ${ENGINE_HEADER} ${ASSETS} ${SCHEMAS}
    ${SCHEMA_DECLARE_OUTPUT_FILES}
    ${SCHEMA_SERD_OUTPUT_FILES}
    ${SCHEMA_ASSET_INFO_FILES}
    ${SCHEMA_BINDING_OUTPUT_FILES})
target_include_directories(${COMMON_NAME} PUBLIC include ../schema_generate)
target_compile_features(${COMMON_NAME} PRIVATE cxx_std_17)
target_link_libraries(${COMMON_NAME}
    PUBLIC
    SDL3::SDL3 SDL3_ttf::SDL3_ttf
    spdlog::spdlog
    stb_image
    imgui
    RapidXML
    stduuid
    tmxlite
    enet
    protobuf::libprotobuf

    Luau.Compiler
    Luau.VM
    LuaBridge)

# TODO: compatible with clang++
if (MSVC)
    target_compile_options(${COMMON_NAME} PRIVATE /bigobj)
endif()

if (TL_ENABLE_PROFILE)
    target_link_libraries(${COMMON_NAME} PUBLIC Tracy::TracyClient)
    target_compile_definitions(${COMMON_NAME} PUBLIC TL_ENABLE_PROFILE)
endif()

target_compile_definitions(${COMMON_NAME}
    PUBLIC $<$<CONFIG:Debug>:TL_DEBUG>
    PRIVATE _CRT_SECURE_NO_WARNINGS)

if (ANDROID)
    target_compile_definitions(${COMMON_NAME} PUBLIC TL_ANDROID)
endif()


