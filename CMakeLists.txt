cmake_minimum_required(VERSION 3.20)

project(TreasureLootre
    LANGUAGES C CXX
    VERSION 0.1.0)

find_package(SDL2 REQUIRED CONFIG)
find_package(SDL2_image REQUIRED CONFIG)
find_package(SDL2_ttf REQUIRED CONFIG)
find_package(SDL2_mixer REQUIRED CONFIG)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(main)
target_sources(main PRIVATE main.cpp)
target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE SDL2::SDL2main SDL2::SDL2)
target_link_libraries(main PRIVATE SDL2_image::SDL2_image)
target_link_libraries(main PRIVATE SDL2_ttf::SDL2_ttf-static)
target_link_libraries(main PRIVATE SDL2_mixer::SDL2_mixer)
