# TreasureLooter
[![Build](https://github.com/VisualGMQ/TreasureLooter/actions/workflows/build.yaml/badge.svg?branch=main)](https://github.com/VisualGMQ/TreasureLooter/actions/workflows/build.yaml)

每周日下午四点B站直播开发，关注[单身剑法传人](https://space.bilibili.com/256768793?spm_id_from=333.1007.0.0)谢谢喵

# Introduce

TreasureLooter is a 2D game made in SDL3

## How To Build

### Build For PC

```bash
cd game
cmake --preset=default
cmake --build cmake-build
```

### Build For Android

First we must build `schema_parser` under PC and generate schema outputs:

```bash
cmake -S game/ -B cmake-build
cmake --build cmake-build --target mobile_preprocess
```

It will generate `game/schema_generate` and `game/scripts/type_hints/tl_schema_types.luau`.
The other type-hint files (`tl_common_types.luau`, `tl_client_types.luau`, `tl_server_types.luau`) are source files under `game/scripts/type_hints`.

Then copy `game` folder to `android/app/jni`.

Copy `game/assets` folder to `android/app/src/main/assets`.

```bash
cp -r game android/app/jni
cp -r game/assets android/app/src/main/assets/
cp -r game/script android/app/src/main/assets/
```

Then use `gradle` to build (or use AndroidStudio)

```bash
cd android
gradlew build
```

## How To Run

Run `TreasureLooter[.exe]` under `game` directory.

Or you can install:

```bash
cmake --build .\cmake-build\ --target install --config Release
```

It will install game to `install` dir. Than you can open `install/TreasureLooter[.exe]` to play.

Also can get a package:

```bash
cmake --build .\cmake-build\ --target package --config Release
```

It will generate `cmake-build/TreasureLooter-<version>-<platform>.zip` for you.

## For Developer

### AI Coding Note

Some projects are entirely written by AI, I didn't review code, so you can ignore them when you read code:

* CollisionEditor: `tools/colision_editor`
* AnimationEditor: `tools/animation_editor`

### Luau developer

For best development experience, open vscode under `TreasureLooter/game`.

You can require [tl_common_types](game/scripts/type_hints/tl_common_types.luau), [tl_client_types](game/scripts/type_hints/tl_client_types.luau), [tl_server_types](game/scripts/type_hints/tl_server_types.luau), [tl_schema_types](game/scripts/type_hints/tl_schema_types.luau), and [imgui_types](scripts/client/imgui_types.luau) to make IDE complete C++ binding functions & classes for you:

```luau
local TLC = require("@game/scripts/type_hints/tl_common_types")
local TLI = require("@game/scripts/type_hints/tl_client_types")
local TLS = require("@game/scripts/type_hints/tl_server_types")
local TLSch = require("@game/scripts/type_hints/tl_schema_types")
local ImGui = require("@game/imgui_types")
```
