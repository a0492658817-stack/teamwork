# Architecture Overview

## Goal

This project is organized to keep game logic independent from platform UI code.

## Directory Structure

- `bin/`: runtime executables
- `build/`: CMake build tree and generated metadata
- `data/`: runtime data files and assets
- `docs/`: project documentation
- `include/`: public headers for core logic
- `lib/`: static/shared libraries from builds
- `src/`: application entry points and core implementation
- `tests/`: unit tests for core modules
- `third_party/`: vendored external libraries

## Layering

- `src/game.c` + `include/game.h`: pure C core logic
- `src/main.c`: platform entry (WinBGIm on Windows, console fallback elsewhere)
- `tests/game_tests.c`: core behavior tests without graphics dependency

## Reveal Flow Ownership

- `player_flip_from_click` (core layer) maps mouse coordinates to a board cell and reveals only when the target is inside the board and still covered.
- `computer_flip` (core layer) reveals one random covered piece.
- `src/main.c` orchestrates turn flow: valid player reveal -> optional delay -> one computer reveal.

## Build Flows

- CMake flow:
  - `cmake -S . -B build`
  - `cmake --build build`
  - `ctest --test-dir build --output-on-failure`
- Makefile flow:
  - `make build`
  - `make test`
  - `make run`
