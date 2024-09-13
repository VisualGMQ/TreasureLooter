# TreasureLooter

A game

## how to build

1. use conan2
    ```bash
    conan install . --build=missing --settings=build_type=Debug 
    ```
    Or you can change `--setting=build_type=Release` to build release version.
2. then run cmake
    ```bash
    cmake -S . -B build/cmake-build -DCMAKE_TOOLCHAIN_FILE=build/generators/conan_toolchain.cmake 
    cmake --build cmake-build
    ```
