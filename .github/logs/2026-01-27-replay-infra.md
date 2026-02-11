# E2E Test Failure Report

## Date
January 27, 2026

## Summary
22 E2E tests failed: 15 action movement/dig cases with empty offset lists, 5 action program parsing/validation cases, and 2 replay frame index mismatch cases with missing error text. No segv stack traces were captured.

## General rerun command
build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/5:E2eAction/ActionE2eTest.Runs/6:E2eAction/ActionE2eTest.Runs/7:E2eAction/ActionE2eTest.Runs/8:E2eAction/ActionE2eTest.Runs/9:E2eAction/ActionE2eTest.Runs/10:E2eAction/ActionE2eTest.Runs/11:E2eAction/ActionE2eTest.Runs/12:E2eAction/ActionE2eTest.Runs/13:E2eAction/ActionE2eTest.Runs/14:E2eAction/ActionE2eTest.Runs/15:E2eAction/ActionE2eTest.Runs/16:E2eAction/ActionE2eTest.Runs/17:E2eAction/ActionE2eTest.Runs/18:E2eAction/ActionE2eTest.Runs/19:E2eAction/ActionE2eTest.Runs/26:E2eAction/ActionE2eTest.Runs/29:E2eAction/ActionE2eTest.Runs/31:E2eAction/ActionE2eTest.Runs/32:E2eAction/ActionE2eTest.Runs/34:E2eReplay/ReplayE2eTest.Runs/8:E2eReplay/ReplayE2eTest.Runs/9

## Failures

### Failure 1: action_move_wall_blocked_1 (E2eAction/ActionE2eTest.Runs/5)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for blocked movement cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/5
- Segv stack trace: not available

### Failure 2: action_move_wall_blocked_2 (E2eAction/ActionE2eTest.Runs/6)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for blocked movement cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/6
- Segv stack trace: not available

### Failure 3: action_move_wall_blocked_3 (E2eAction/ActionE2eTest.Runs/7)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for blocked movement cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/7
- Segv stack trace: not available

### Failure 4: action_move_wall_blocked_4 (E2eAction/ActionE2eTest.Runs/8)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for blocked movement cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/8
- Segv stack trace: not available

### Failure 5: action_move_wall_blocked_5 (E2eAction/ActionE2eTest.Runs/9)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for blocked movement cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/9
- Segv stack trace: not available

### Failure 6: action_move_wall_disabled_1 (E2eAction/ActionE2eTest.Runs/10)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for wall-disabled movement cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/10
- Segv stack trace: not available

### Failure 7: action_move_wall_disabled_2 (E2eAction/ActionE2eTest.Runs/11)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for wall-disabled movement cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/11
- Segv stack trace: not available

### Failure 8: action_move_wall_disabled_3 (E2eAction/ActionE2eTest.Runs/12)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for wall-disabled movement cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/12
- Segv stack trace: not available

### Failure 9: action_move_wall_disabled_4 (E2eAction/ActionE2eTest.Runs/13)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for wall-disabled movement cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/13
- Segv stack trace: not available

### Failure 10: action_move_wall_disabled_5 (E2eAction/ActionE2eTest.Runs/14)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for wall-disabled movement cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/14
- Segv stack trace: not available

### Failure 11: action_dig_wall_1 (E2eAction/ActionE2eTest.Runs/15)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for dig-wall cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/15
- Segv stack trace: not available

### Failure 12: action_dig_wall_2 (E2eAction/ActionE2eTest.Runs/16)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for dig-wall cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/16
- Segv stack trace: not available

### Failure 13: action_dig_wall_3 (E2eAction/ActionE2eTest.Runs/17)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for dig-wall cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/17
- Segv stack trace: not available

### Failure 14: action_dig_wall_4 (E2eAction/ActionE2eTest.Runs/18)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for dig-wall cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/18
- Segv stack trace: not available

### Failure 15: action_dig_wall_5 (E2eAction/ActionE2eTest.Runs/19)
- Failure: offsets.size() == 0 (expected >= 5)
- Issue: No wall-adjacent offsets were found for dig-wall cases.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/19
- Segv stack trace: not available

### Failure 16: action_program_valid_2 (E2eAction/ActionE2eTest.Runs/26)
- Failure: has_code is false (expected true)
- Issue: Parser rejected "MOVE DOWN" with "expecting no args" / "Additional characters remained: MOVE" and cleared machine code.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/26
- Segv stack trace: not available

### Failure 17: action_program_valid_5 (E2eAction/ActionE2eTest.Runs/29)
- Failure: has_code is false (expected true)
- Issue: Parser rejected "LDI" as an invalid command and cleared machine code.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/29
- Segv stack trace: not available

### Failure 18: action_program_invalid_2 (E2eAction/ActionE2eTest.Runs/31)
- Failure: has_code is true (expected false)
- Issue: Parser accepted an invalid program line ("MOVE").
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/31
- Segv stack trace: not available

### Failure 19: action_program_invalid_3 (E2eAction/ActionE2eTest.Runs/32)
- Failure: has_code is true (expected false)
- Issue: Parser accepted an invalid program line ("ADD A").
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/32
- Segv stack trace: not available

### Failure 20: action_program_invalid_5 (E2eAction/ActionE2eTest.Runs/34)
- Failure: C++ exception "_Map_base::at" thrown in test body
- Issue: "JMP" without a direction logged "NEED DIRECTION DEFINED FOR JMP COMMAND" before the exception.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/34
- Segv stack trace: not available

### Failure 21: replay_frame_index_mismatch_4 (E2eReplay/ReplayE2eTest.Runs/8)
- Failure: expected "Replay frame mismatch" message not found in error text
- Issue: Replay validation did not include the frame mismatch error substring.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eReplay/ReplayE2eTest.Runs/8
- Segv stack trace: not available

### Failure 22: replay_frame_index_mismatch_5 (E2eReplay/ReplayE2eTest.Runs/9)
- Failure: expected "Replay frame mismatch" message not found in error text
- Issue: Replay validation did not include the frame mismatch error substring.
- Rerun command: build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eReplay/ReplayE2eTest.Runs/9
- Segv stack trace: not available
