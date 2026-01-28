# E2E Action: wall offsets missing

## Summary
Multiple E2E action tests report no wall-adjacent offsets collected near the player. Failures show `offsets.size() == 0` where >= 5 is expected.

## Failures
**Blocked movement**
- E2eAction/ActionE2eTest.Runs/5 (action_move_wall_blocked_1)
- E2eAction/ActionE2eTest.Runs/6 (action_move_wall_blocked_2)
- E2eAction/ActionE2eTest.Runs/7 (action_move_wall_blocked_3)
- E2eAction/ActionE2eTest.Runs/8 (action_move_wall_blocked_4)
- E2eAction/ActionE2eTest.Runs/9 (action_move_wall_blocked_5)

**Walls disabled**
- E2eAction/ActionE2eTest.Runs/10 (action_move_wall_disabled_1)
- E2eAction/ActionE2eTest.Runs/11 (action_move_wall_disabled_2)
- E2eAction/ActionE2eTest.Runs/12 (action_move_wall_disabled_3)
- E2eAction/ActionE2eTest.Runs/13 (action_move_wall_disabled_4)
- E2eAction/ActionE2eTest.Runs/14 (action_move_wall_disabled_5)

**Dig wall**
- E2eAction/ActionE2eTest.Runs/15 (action_dig_wall_1)
- E2eAction/ActionE2eTest.Runs/16 (action_dig_wall_2)
- E2eAction/ActionE2eTest.Runs/17 (action_dig_wall_3)
- E2eAction/ActionE2eTest.Runs/18 (action_dig_wall_4)
- E2eAction/ActionE2eTest.Runs/19 (action_dig_wall_5)

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
Wall-adjacent offsets should be collected (`offsets.size() >= 5`).

## Actual
`offsets.size() == 0` and tests fail. No segv stack trace available.

## Notes
Source report: .github/logs/e2e_failure_report_2026-01-27.md
