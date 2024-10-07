cmake_minimum_required(VERSION 3.20)

project(TreasureLootre
    LANGUAGES C CXX
    VERSION 0.1.0)

aux_source_directory(src GAME_SRC)
aux_source_directory(src/3rdlib/imgui GAME_SRC)
aux_source_directory(src/3rdlib/tinyxml2 GAME_SRC)
aux_source_directory(src/input GAME_SRC)
aux_source_directory(src/controller GAME_SRC)


if (ANDROID)
    add_library(main SHARED)
    find_library(SDL2 SDL2)
    find_library(SDL2_image SDL2_image)
    find_library(SDL2_ttf SDL2_ttf)
    find_library(SDL2_mixer SDL2_mixer)

    target_compile_features(main PRIVATE cxx_std_17)
    target_link_libraries(main PRIVATE SDL2)
    target_link_libraries(main PRIVATE SDL2_image)
    target_link_libraries(main PRIVATE SDL2_ttf)
    target_link_libraries(main PRIVATE SDL2_mixer)
    target_compile_definitions(main PRIVATE TL_ANDROID)
else()
    find_package(SDL2 REQUIRED CONFIG)
    find_package(SDL2_image REQUIRED CONFIG)
    find_package(SDL2_ttf REQUIRED CONFIG)
    find_package(SDL2_mixer REQUIRED CONFIG)

    add_executable(main)
    target_link_libraries(main
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )
    target_link_libraries(main PRIVATE $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)
    target_link_libraries(main PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
    target_link_libraries(main PRIVATE $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)
endif()

target_sources(main PRIVATE ${GAME_SRC})
target_compile_features(main PRIVATE cxx_std_17)
target_include_directories(main PRIVATE src)
target_compile_definitions(main PRIVATE _CRT_SECURE_NO_WARNINGS)

