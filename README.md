# DONJON

Topdown dungeon crawler roguelite built in C++ with raylib.

## Current Setup

- `raylib` is added through CMake `FetchContent` in `CMakeLists.txt`.
- Main entry point: `src/main.cpp`.

## Build Requirements (Windows)

- CMake 3.21+
- C++ compiler (Visual Studio Build Tools / MSVC or MinGW)

If CMake is missing, install it with:

```powershell
winget install Kitware.CMake
```

## Build

```powershell
cmake -S . -B build
cmake --build build -j
```

## Current Prototype Features

- Procedural dungeon generation (rooms + corridors)
- Turn-based movement using `WASD` or arrow keys
- Enemy movement and melee combat
- XP, level-ups, and scaling difficulty per floor
- Stairs (`>`) to descend and generate a new floor
- Permadeath with restart (`R` after death)

## Optional Audio Assets

The game now supports background music + SFX via raylib audio. Place files in `assets/audio/`:

- `bgm.ogg` (looped background music)
- `sfx_attack.wav`
- `sfx_hurt.wav`
- `sfx_gold.wav`
- `sfx_interact.wav`
- `sfx_stairs.wav`
- `sfx_death.wav`

If files are missing, the game runs normally without audio for those channels.

## Controls

- `WASD` / Arrow keys: move
- Move into an enemy to attack
- Reach stairs (`>`) to descend
- `R`: restart after death

## VS Code Presets Workflow

`CMakePresets.json` includes ready-to-use presets:

- Configure: `debug`, `release`
- Build: `build-debug`, `build-release`

In VS Code (with CMake Tools extension):

1. Run `CMake: Select Configure Preset` and choose `Debug` or `Release`.
2. Run `CMake: Configure`.
3. Run `CMake: Select Build Preset` and choose `Build Debug` or `Build Release`.
4. Run `CMake: Build`.
