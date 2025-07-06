file(GLOB_RECURSE SRC ./*.cpp ./backends/*.cpp ./misc/*.cpp)
file(GLOB_RECURSE HEADER ./*.h ./backends/*.h ./misc/*.h)

add_library(imgui STATIC)
target_sources(imgui PRIVATE ${SRC} ${HEADER})
target_include_directories(imgui PUBLIC ./)
target_link_libraries(imgui PRIVATE SDL3::SDL3)