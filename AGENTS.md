# Bomberman — Agent Guide

## Dev environment
- Uses [devenv](https://devenv.sh) (Nix) for reproducible toolchain. Run `direnv allow` or `devenv shell` to activate.
- Provided tools: cmake, clang-tools (clangd, clang-format, clang-tidy), raylib, bear.

## Build & run
```sh
cmake -S . -B build -G "Unix Makefiles"
cmake --build build
./build/bomber
```

## Debug mode
Set `DEBUG=1` env var to enable debug log output:
```sh
DEBUG=1 ./build/bomber
```

## Project structure
- `src/` — all source, `.cpp`/`.hpp` pairs, entrypoint in `main.cpp`
- `assets/` — game textures (characters/, effects/, items/, ui/)
- `build/` — CMake out-of-source build dir (git-ignored)
- Single binary target: `bomber`

## Code conventions
- C++20, no exceptions or RTTI used
- Classes with static methods for singletons (Renderer, InputHandler, BotController)
- `#pragma once` headers
- German comments in some legacy files
- Grid: 15×15, `Position` uses `{x, y}` int coordinates
- Up to 4 players (`kMaxPlayers`), player 0 = human, rest = bots
- Indentation: spaces, 2-wide per `.editorconfig`

## Notable
- No test framework, no CI, no linter/formatter config
- Uses raylib for windowing, input, rendering, textures
- `GetFrameTime()` for delta-time, not fixed timestep
- `SetExitKey(KEY_NULL)` — ESC doesn't close window
- Character animations loaded from `assets/characters/` numbered directories
