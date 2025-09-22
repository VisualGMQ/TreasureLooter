add_library(optick STATIC)

file(GLOB_RECURSE SRC *.cpp)
file(GLOB_RECURSE HEADER *.hpp)
target_sources(optick PRIVATE ${SRC} ${HEADER})
target_include_directories(optick PUBLIC .)