# Unit/E2E Test Failure Report - 2026-01-27

## Build
- Command: cmake --build /home/wwodrich/projects/ants/build_tests
- Result: Success

## Test Runs
- Command: cd /home/wwodrich/projects/ants/build_tests && ctest --output-on-failure
- Result: Failures observed

### Failing Test Targets (from full run)
- HardwareTokenParserTests (Failed)
- HardwareParserTests (Failed)
- E2ETests (SEGFAULT)

### Reruns
- UtilsUnitTests: Passed on rerun (no failures observed)
- HardwareTokenParserTests: Failed (see details below)
- HardwareParserTests: Failed (see details below)
- E2ETests: Failed with multiple assertion failures and reported SEGFAULT

## Failure Details

### HardwareTokenParserTests
- Multiple invalid integer cases throw std::invalid_argument or std::out_of_range from std::stoi in TokenParser::integer.
- Multiple invalid signed byte cases throw std::invalid_argument/std::out_of_range in TokenParser::get_signed_byte and do not set Status::p_err.
- Representative failures:
  - Expected no throw, got std::invalid_argument("stoi")
  - Expected no throw, got std::out_of_range("stoi")
  - Status::p_err remained false for invalid signed byte inputs

### HardwareParserTests
- Many invalid program cases throw std::invalid_argument or std::out_of_range (stoi) instead of setting Status::p_err.
- Jump parsing with missing/invalid labels throws std::out_of_range("_Map_base::at") from label lookup.
- Some invalid register tokens only log errors but still parse successfully (Status::p_err remains false).

### E2ETests (SEGFAULT reported by ctest)
- Action E2E failures:
  - Expected offsets.size() >= 5, actual 0 for action_move_wall_disabled_3..5
  - Expected offsets.size() >= 5, actual 0 for action_dig_wall_1..5
  - action_program_valid_2 and action_program_valid_5: has_code() false after parse (expected true)
  - action_program_invalid_2 and action_program_invalid_3: has_code() true after parse (expected false)
  - action_program_invalid_5: exception thrown (_Map_base::at) after NEED DIRECTION DEFINED FOR JMP COMMAND
- Replay E2E failures:
  - replay_frame_index_mismatch_4 and replay_frame_index_mismatch_5: error message did not include "Replay frame mismatch"

## Notes
- E2ETests reported as SEGFAULT by ctest despite assertion-style failures in output. No explicit stack trace was provided.
