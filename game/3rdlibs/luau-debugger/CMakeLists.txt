cmake_minimum_required(VERSION 3.20)

project(luau_debugger)

set(CPP_DAP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/cppdap)
set(LUAU_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../luau)

add_subdirectory(cppdap)
add_subdirectory(debugger)