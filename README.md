# teamwork

## Build

The project now separates pure C core game logic from the platform UI layer.

### CMake

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

### Makefile

```bash
make build
make test
make run
```

On Windows, the `teamwork` executable uses WinBGIm.

On macOS, the `teamwork` executable now uses SDL2 for window rendering when SDL2 is available.

```bash
brew install sdl2
cmake -S . -B build
cmake --build build
./bin/teamwork
```

If SDL2 is not found on macOS, the app falls back to the console mode so the core logic and tests still compile.

## Layout

- `bin/` contains runtime executables.
- `build/` contains CMake cache and generated build files.
- `data/` contains runtime assets.
- `docs/` contains architecture and project docs.
- `include/` contains the public core API.
- `lib/` contains generated static/shared libraries.
- `src/` contains core implementation and platform entry points.
- `tests/` contains unit tests for board setup and reveal logic.

## Notes

- Runtime images are loaded from `data/images`.

## Reveal Rules

- Player action: click one covered piece to reveal it.
- Invalid player action: clicking outside the board or clicking an already revealed piece does not change game state.
- After a valid player reveal, computer reveals exactly one covered piece.
- The game ends when all pieces are revealed.
- Core helper API for player click handling is `player_flip_from_click` in `game.h`.

## CI

GitHub Actions runs the following on push and pull request:

- CMake configure/build
- CTest execution
- Makefile `make test` flow