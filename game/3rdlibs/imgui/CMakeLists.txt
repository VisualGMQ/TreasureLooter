file(GLOB_RECURSE SRC ./*.cpp ./backends/*.cpp ./misc/*.cpp)
file(GLOB_RECURSE HEADER ./*.h ./backends/*.h ./misc/*.h)

add_library(imgui INTERFACE)
target_sources(imgui INTERFACE ${SRC} ${HEADER})
target_include_directories(imgui INTERFACE ./)
# target_link_libraries(imgui PRIVATE SDL3::SDL3)
target_compile_features(imgui INTERFACE cxx_std_17)