# E2E Replay: Large Frame Incomplete Test Failures

## Summary
E2E replay tests failed to complete for large mixed event frames.

## Failing Tests
- E2eReplay/ReplayE2eTest.Runs/47: replay_large_frame_3
- E2eReplay/ReplayE2eTest.Runs/48: replay_large_frame_4
- E2eReplay/ReplayE2eTest.Runs/49: replay_large_frame_5

## Log Excerpt
```
Value of: result.done
  Actual: false
Expected: true
```

## Impact
Replay system does not properly complete for large event frames.

## Source
See .github/logs/full_save_restore_run_*.log for details.