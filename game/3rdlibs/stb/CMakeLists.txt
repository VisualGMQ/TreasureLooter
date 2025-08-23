add_library(stb_image STATIC)
target_sources(stb_image PRIVATE stb_image.h stb_image.cpp)
target_include_directories(stb_image PUBLIC .)

add_library(stb_vorbis STATIC)
target_sources(stb_vorbis PRIVATE stb_vorbis.h stb_vorbis.cpp)
target_include_directories(stb_vorbis PUBLIC .)