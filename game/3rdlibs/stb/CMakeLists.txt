add_library(stb_image STATIC stb_image.cpp)
target_include_directories(stb_image PUBLIC .)

add_library(stb_truetype STATIC stb_truetype.cpp)
target_include_directories(stb_truetype PUBLIC .)