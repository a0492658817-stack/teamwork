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

On Windows, the `teamwork` executable uses WinBGIm. On non-Windows platforms, the same entry point builds a console fallback so the core logic and tests still compile.

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

## CI

GitHub Actions runs the following on push and pull request:

- CMake configure/build
- CTest execution
- Makefile `make test` flow