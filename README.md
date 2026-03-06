# DONJON

Topdown D-Crawler built in C++ with raylib.

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

## VS Code Presets Workflow

`CMakePresets.json` includes ready-to-use presets:

- Configure: `debug`, `release`
- Build: `build-debug`, `build-release`

In VS Code (with CMake Tools extension):

1. Run `CMake: Select Configure Preset` and choose `Debug` or `Release`.
2. Run `CMake: Configure`.
3. Run `CMake: Select Build Preset` and choose `Build Debug` or `Build Release`.
4. Run `CMake: Build`.
