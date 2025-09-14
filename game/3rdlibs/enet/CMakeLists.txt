add_library(enet STATIC)
target_sources(enet PRIVATE enet.h enet.cpp)
target_include_directories(enet PUBLIC .)