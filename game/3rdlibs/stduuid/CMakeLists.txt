file(GLOB_RECURSE HEADERS ./*.h ./gls/.*)
add_library(stduuid INTERFACE ${HEADERS})
target_include_directories(stduuid INTERFACE .)
target_compile_features(stduuid INTERFACE cxx_std_17)