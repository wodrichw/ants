# E2E Replay: missing "Replay frame mismatch" message

## Summary
Replay validation failures do not include the expected "Replay frame mismatch" substring in the error message.

## Failures
- E2eReplay/ReplayE2eTest.Runs/8 (replay_frame_index_mismatch_4)
- E2eReplay/ReplayE2eTest.Runs/9 (replay_frame_index_mismatch_5)

## Steps to Reproduce
1. Configure tests:
   ```
   cmake -S /home/wwodrich/projects/ants -B /home/wwodrich/projects/ants/build_tests -DUNIT_TEST=ON -DE2E_TESTS=ON
   cmake --build /home/wwodrich/projects/ants/build_tests
   ```
2. Run one failing case:
   ```
   ./build_tests/tests/e2e_tests/test_e2e --gtest_filter=E2eReplay/ReplayE2eTest.Runs/<N>
   ```

## Expected
`result.error.message` should contain "Replay frame mismatch".

## Actual
Error text does not contain the expected substring.

## Notes
Source report: .github/logs/e2e_failure_report_2026-01-27.md
