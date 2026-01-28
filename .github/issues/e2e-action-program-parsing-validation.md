# E2E Action: program parsing/validation regressions

## Summary
E2E action tests show parser inconsistencies: valid programs are rejected, invalid programs are accepted, and a missing direction for `JMP` throws an exception.

## Failures
**Valid program rejected**
- E2eAction/ActionE2eTest.Runs/26 (action_program_valid_2): "MOVE DOWN" rejected (expects no args).
- E2eAction/ActionE2eTest.Runs/29 (action_program_valid_5): "LDI" rejected as invalid command.

**Invalid program accepted**
- E2eAction/ActionE2eTest.Runs/31 (action_program_invalid_2): "MOVE" accepted.
- E2eAction/ActionE2eTest.Runs/32 (action_program_invalid_3): "ADD A" accepted.

**Exception**
- E2eAction/ActionE2eTest.Runs/34 (action_program_invalid_5): exception `_Map_base::at` after "JMP" without direction.

## Steps to Reproduce
1. Configure tests:
   ```
   cmake -S /home/wwodrich/projects/ants -B /home/wwodrich/projects/ants/build_tests -DUNIT_TEST=ON -DE2E_TESTS=ON
   cmake --build /home/wwodrich/projects/ants/build_tests
   ```
2. Run one failing case:
   ```
   ./build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eAction/ActionE2eTest.Runs/<N>
   ```

## Expected
- Valid programs should parse successfully.
- Invalid programs should be rejected.
- Missing direction should produce a clean validation error (no exception).

## Actual
- Parser rejects valid inputs and accepts invalid ones.
- Missing direction for `JMP` throws `_Map_base::at`.

## Notes
Source report: .github/logs/e2e_failure_report_2026-01-27.md
