# E2E Replay: Frame Index Mismatch Test Failures

## Summary
Some E2E replay tests failed due to frame index mismatches not being detected as errors.

## Failing Tests
- E2eReplay/ReplayE2eTest.Runs/8: replay_frame_index_mismatch_4
- E2eReplay/ReplayE2eTest.Runs/9: replay_frame_index_mismatch_5

## Impact
Replay system does not properly detect or report frame index mismatches.

## Source
See .github/logs/full_save_restore_run_*.log for details.