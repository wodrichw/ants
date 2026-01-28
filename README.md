# Build and Install
## Dependencies
Make sure you have the followng dependencies installed on your machine before installing the game
- Cmake `sudo apt install cmake -y`
- A c++ compiler `sudo apt install build-essential`
- SDL
```
sudo apt install libsdl2-dev libsdl2-2.0-0 -y
sudo apt install libjpeg-dev libwebp-dev libtiff5-dev libsdl2-image-dev libsdl2-image-2.0-0 -y
sudo apt install libmikmod-dev libfishsound1-dev libsmpeg-dev liboggz2-dev libflac-dev libfluidsynth-dev libsdl2-mixer-dev libsdl2-mixer-2.0-0 -y
sudo apt install libfreetype6-dev libsdl2-ttf-dev libsdl2-ttf-2.0-0 -y
```
- X11 `sudo apt install libx11-dev libxft-dev libxext-dev -y`
- Wayland `sudo apt install libwayland-dev libxkbcommon-dev libegl1-mesa-dev -y`
- iBus `sudo apt install libibus-1.0-dev -y`


## Install instructions
1. Get vcpkg up and running by doing `git submodule init; git submodule update; ./vcpkg/bootstrap-vcpkg.sh ; ./vcpkg/vcpkg install `
2. Get cmake to generate make file from CMakeLists.txt `[ -d "./build" ] || mkdir build; cd build; cmake -DOPT=1 ..`
3. Build ants executable `make` -- this currently builds ants with debug symbols as specified in CMakeLists.txt file

## Easy install
- run `./install_dependencies.sh`
- after this, you can build ants with `make`

## Debug build with logs
For a Debug build with maximum debuggability and JSON logging of build/test issues:
```
./scripts/build_debug_with_logs.sh
```

This writes a timestamped JSON Lines log to `.github/logs` and continues through all steps, logging any errors.

## E2E tests
- Test sources:
  - [tests/e2e_tests/replay_e2e_tests.cpp](tests/e2e_tests/replay_e2e_tests.cpp)
  - [tests/e2e_tests/action_e2e_tests.cpp](tests/e2e_tests/action_e2e_tests.cpp)
  - Helpers: [tests/e2e_tests/e2e_helpers.hpp](tests/e2e_tests/e2e_helpers.hpp), [tests/e2e_tests/e2e_helpers.cpp](tests/e2e_tests/e2e_helpers.cpp)
- Replay fixtures (generated): [tests/test_assets/replays/generated](tests/test_assets/replays/generated)

### Run E2E tests
Build with tests enabled, then run the E2E target:
```
cmake -S . -B build_tests -DUNIT_TEST=ON
cmake --build build_tests
ctest --test-dir build_tests -R E2ETests --output-on-failure
```

## Python bindings (local build)
This repository does not ship a pip-installable package. To use the Python bindings, build the extension module and add it to your Python path.

### Install Python dependencies
```
python3 -m pip install -r requirements.txt
```

### Build the Python extension
```
cmake -S . -B build_python -DCMAKE_BUILD_TYPE=Release
cmake --build build_python --target ant_core
```

This produces the module at `build_python/src/py_wrapper/ant_core.so`.

## Run from a Python file
Use the helper script in `scripts/run_game.py`:
```
python3.10 scripts/run_game.py
```

If you want your own entry point, base it on [scripts/run_game.py](scripts/run_game.py).

Note: the prebuilt module in this repo is compiled for Python 3.10. If you are using a different Python version, rebuild the extension with your current interpreter (see the build steps above).

## Development Progress

### Source Code Structure

- **src/main.cpp**: Entry point for the C++ application. Creates an `AntGameFacade` and runs its update loop.
- **src/core/app/**: Contains the main application logic:
  - **facade.hpp/cpp**: Defines `AntGameFacade`, the main interface for running and updating the game engine.
  - **engine.hpp/cpp**: Core game engine responsible for initialization, updates, and rendering.
  - **engine_state.hpp/cpp**: Manages the current game state, including entities, map, and event systems.
  - **game_mode.hpp/cpp**: Defines different game modes (primary gameplay, editor) and their event handling.
  - **game_state.hpp/cpp**: Represents the persistent state of the game.
  - **arg_parse.hpp/cpp**: Handles command-line or configuration argument parsing.
  - **globals.hpp**: Contains global constants and settings.
- **src/core/entity/**: Implements in-game entities:
  - **ant.hpp/cpp**: Worker ant logic and data.
  - **entity_manager.hpp/cpp**: Manages all entities in the game.
  - **building.hpp/cpp**: Structures/buildings in the game.
  - **inventory.hpp/cpp**: Inventory system for entities.
  - **scents.hpp/cpp**: Scent-tracking for ant navigation.
  - **rect.hpp/cpp**: Rectangle utility for positioning and collision.
- **src/core/map/**: Handles the game world and map:
  - **map.hpp/cpp**: Main map logic and data.
  - **world.hpp/cpp**: Manages multiple levels or regions.
  - **manager.hpp/cpp**: High-level map management.
  - **builder.hpp/cpp**: Map/level generation.
  - **window.hpp/cpp**: Viewport/window into the map.
  - **section_data.hpp/cpp**: Data for map sections.
- **src/core/ui/**: User interface and rendering:
  - **render.hpp/cpp**: Rendering logic for the game.
  - **event_system.hpp/cpp**: Handles user input and events.
  - **text_editor.hpp/cpp**: In-game text editor.
  - **ui_handlers.hpp/cpp**: UI event handlers.
  - **layoutbox.hpp/cpp**: UI layout management.
  - **colors.hpp**: Color definitions.
  - **debug_graphics.hpp/cpp**: Debug rendering utilities.
- **src/core/hardware/**: Simulates in-game hardware and programmable logic:
  - **program_executor.hpp/cpp**: Executes programs for worker ants.
  - **software_manager.hpp/cpp**: Manages software for ants.
  - **token_parser.hpp/cpp**, **parser.hpp/cpp**: Parsing for ant programs.
  - **op_def.hpp/cpp**: Operation definitions for the ant virtual machine.
  - **command_config.hpp/cpp**, **command_parsers.hpp/cpp**, **command_compilers.hpp**: Command and instruction handling.
  - **hardware_manager.hpp/cpp**: Manages all hardware components.
- **src/core/utils/**: Utility code:
  - **thread_pool.hpp/cpp**: Thread pool for async jobs.
  - **serializer.hpp/cpp**: Serialization utilities.
  - **string.hpp/cpp**: String utilities.
  - **math.hpp/cpp**: Math utilities.
  - **status.hpp/cpp**: Status/result types.
- **src/core/os/**: **terminal.hpp**: Terminal/console handling.
- **src/py_wrapper/**: Python bindings for the core engine using pybind11:
  - **main.py**: Python entry point, runs the game loop.
  - **py_wrapper.cpp**: Exposes `AntGameFacade` to Python.
  - **stubs/ant_core.pyi**: Type stubs for Python autocompletion.
- **src/proto/**: Protocol buffer definitions for serialization:
  - **map.proto, entity.proto, hardware.proto, engine.proto, ui.proto, utils.proto, app.proto**: Define the structure of game data for serialization and networking.
- **src/benchmark/**: **ant_bench.cpp**: Benchmarks for game performance using Google Benchmark.

