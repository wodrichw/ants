# CTest Failures (2026-02-07)

Preset: windows-ninja-msvc-debug-trace
Command: ctest --test-dir build_tests_ninja_msvc_debug_trace --output-on-failure

## Summary
- 4 tests failed out of 24 (83% passed).

## Failed tests
1. HardwareProgramExecutorTests (Failed)
2. AppUnitTests (Exit code 0xc0000409)
3. SaveRestoreUnitTests (Exit code 0xc0000409)
4. E2ETests (Exit code 0xc0000409)

## Notable output (E2ETests)
- Assertion failed: Cannot access value of empty optional
  at: C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.42.34433\include\optional(380)
