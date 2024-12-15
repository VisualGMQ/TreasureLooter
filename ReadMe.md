[![Build](https://github.com/VisualGMQ/TreasureLooter/actions/workflows/build.yaml/badge.svg)](https://github.com/VisualGMQ/TreasureLooter/actions/workflows/build.yaml)

# TreasureLooter

A game

## Dependencies

* [vcpkg](https://vcpkg.io/): use to install SDL
* `python3`(optional): use to run tools(need when you want develop this game)

## Assets

* [ninja-adventure-asset-pack](https://pixel-boy.itch.io/ninja-adventure-asset-pack): CC0

## How to build

### Windows/MacOS/Linux

1. enter `game` folder
2. use cmake to build
    ```bash
    cmake --preset=default
    cmake --build cmake-build
    ```

### Emscripten

1. enter `game` folder
2. config cmake:
    ```bash
    emcmake cmake -S . -B cmake-build
    ```
3. use cmake to build
    ```bash
    cmake --build cmake-build
    ```
4. run game
    ```bash
    cd cmake-build
    emrun main.html
    ```

### Android

1. fetch submodules:
    ```bash
    git submodule update --init --recursive
    ```
2. copy `game` folder to `android/app/jni`
3. copy `game/assets` folder to `android/app/src/main/assets`(copy whole dir, not copy contents of `assets` to `android/app/src/main/assets`)
4. use `Android Studio` to build or run `./android/gradlew build`

## How to develop

* all game assets & codes are under `game` folder, so you should code & run under `game` folder
* after you change assets, run `python3 tool/asset_fixer.py` or `cmake --build cmake-build --target asset_fix` to fix your asset
* upload fixed assets to github

### What `tool/asset_fixer.py` do

some assets:

1. `Tiled` exported tilemap data

contains incompatible data with our asset definitions. `asset_fixer.py` will fix them.