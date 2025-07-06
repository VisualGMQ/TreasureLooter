cmake_minimum_required(VERSION 3.20)

project(TreasureLooter
    VERSION 0.0.1)

add_subdirectory(3rdlibs)

file(GLOB_RECURSE SRC ./src/*.cpp ./src/*.c)
file(GLOB_RECURSE HEADER ./include/*.hpp ./include/*.h)
file(GLOB_RECURSE ASSETS ./assets/*)

set(GAME_NAME ${PROJECT_NAME})
if (ANDROID)
    set(GAME_NAME main)
    add_library(${GAME_NAME} SHARED)
else()
    add_executable(${GAME_NAME})
endif()

target_sources(${GAME_NAME} PRIVATE ${SRC} ${HEADER} ${ASSETS})
target_include_directories(${GAME_NAME} PRIVATE include)
target_compile_features(${GAME_NAME} PRIVATE cxx_std_17)
target_link_libraries(${GAME_NAME} PRIVATE SDL3::SDL3 spdlog::spdlog stb_image imgui)