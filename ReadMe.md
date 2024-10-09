# TreasureLooter

A game

## how to build

### Windows/MacOS/Linux

1. enter `game` folder
2. Please install `vcpkg`(Use to install SDL)
3. use cmake to build
    ```bash
    cmake --preset=default
    cmake --build cmake-build
    ```

### Android

1. fetch submodules:
    ```bash
    git submodule update --init --recursive
    ```
2. copy `game` folder to `android/app/jni`
3. copy `game/assets` folder to `android/app/src/main/assets`(copy whole dir, not copy contents of `assets` to `android/app/src/main/assets`)
4. use `Android Studio` to build or run `./android/gradlew build`
