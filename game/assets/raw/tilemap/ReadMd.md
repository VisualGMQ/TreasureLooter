# How to make TileMap

Use `Tiled` (I use version 1.11.0) to edit your tilemap:

1. All `TileSet` **must** be embed(due to `tileson` library limit)
2. Export tilemap as `json`
3. Only support `finite` tilemap(but you can edit your tilemap in `infinite` mode, and change to `finite` when you export tilemap)
4. Images in exported tilemap is not correct. Change these image path in exported `.json` file to registered name in `assets/assets.xml` directly.