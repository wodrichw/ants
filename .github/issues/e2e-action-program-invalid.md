# E2E Action: Program Invalid Test Failures

## Summary
Some E2E action tests failed due to invalid program handling.

## Failing Tests
- E2eAction/ActionE2eTest.Runs/31: action_program_invalid_2
- E2eAction/ActionE2eTest.Runs/32: action_program_invalid_3
- E2eAction/ActionE2eTest.Runs/33: action_program_invalid_4
- E2eAction/ActionE2eTest.Runs/34: action_program_invalid_5

## Impact
Action system does not properly handle or report invalid programs.

## Source
See .github/logs/full_save_restore_run_*.log for details.