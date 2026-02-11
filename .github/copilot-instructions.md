# Build

Use CMake presets.

## Windows

1. Configure:
   - cmake --preset windows-ninja-msvc-debug-trace
2. Build:
   - cmake --build --preset windows-ninja-msvc-debug-trace

Notes:
- Run from a VS Dev Prompt (x64 host/target) if the toolchain is not already configured.

## Linux

1. Configure:
   - cmake --preset ubuntu-debug-trace
2. Build:
   - cmake --build --preset ubuntu-debug-trace
