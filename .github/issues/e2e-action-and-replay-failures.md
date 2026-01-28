# E2ETests failing (actions, program parsing, replay mismatch, reported segfault)

## Summary
E2E action tests fail due to missing offsets and inconsistent program parsing outcomes. Replay mismatch tests fail due to missing expected error message. CTest also reports a SEGFAULT for E2ETests.

## Failed Tests
- E2ETests
  - E2eAction/ActionE2eTest.Runs/12..19: offsets.size() == 0, expected >= 5
  - E2eAction/ActionE2eTest.Runs/26: has_code() false for action_program_valid_2
  - E2eAction/ActionE2eTest.Runs/29: has_code() false for action_program_valid_5
  - E2eAction/ActionE2eTest.Runs/31..32: has_code() true for action_program_invalid_2/3
  - E2eAction/ActionE2eTest.Runs/34: _Map_base::at exception after missing jump label
  - E2eReplay/ReplayE2eTest.Runs/8..9: error message missing "Replay frame mismatch"

## Evidence
- offsets.size() == 0 when wall-disabled move/dig actions run.
- Parser throws _Map_base::at on missing label for JMP.
- Replay mismatch error message not present in result.error.message.
- ctest reports E2ETests as SEGFAULT despite assertion-style failures.

## Suspected Areas
- tests/e2e_tests/action_e2e_tests.cpp
- tests/e2e_tests/replay_e2e_tests.cpp
- src/core/hardware/parser.cpp / command_parsers.cpp
- Replay error message formatting in replay loading logic

## Suggested Fix Direction
- Investigate action offset collection and wall-disabled map updates.
- Ensure invalid program parsing clears code and sets Status.
- Ensure replay mismatch errors include standardized message.
- Re-run with sanitizer to confirm real segfault vs ctest reporting.
