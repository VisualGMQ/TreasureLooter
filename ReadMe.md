# TreasureLooter
[![Build](https://github.com/VisualGMQ/TreasureLooter/actions/workflows/build.yaml/badge.svg?branch=main)](https://github.com/VisualGMQ/TreasureLooter/actions/workflows/build.yaml)

每周日下午四点B站直播开发，关注[单身剑法传人](https://space.bilibili.com/256768793?spm_id_from=333.1007.0.0)谢谢喵

# Introduce

TreasureLooter is a 2D game made in SDL3

## How To Build

```bash
cmake --preset=default
cmake --build cmake-build
```

## How To Run

Run `TreasureLooter[.exe]` under current directory.

Or you can install:

```bash
cmake --build .\cmake-build\ --target install --config Release
```

it will install game to `install` dir. Than you can open `install/TreasureLooter[.exe]` to play.

Also can get a package:

```bash
cmake --build .\cmake-build\ --target package --config Release
```

it will generate `cmake-build/TreasureLooter-<version>-<platform>.zip` for you.