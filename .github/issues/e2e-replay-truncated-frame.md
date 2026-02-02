# E2E Replay: Truncated Frame Test Failures

## Summary
E2E replay tests failed to detect truncated replay frame bodies as errors.

## Failing Tests
- E2eReplay/ReplayE2eTest.Runs/35: replay_truncated_frame_1
- E2eReplay/ReplayE2eTest.Runs/36: replay_truncated_frame_2
- E2eReplay/ReplayE2eTest.Runs/37: replay_truncated_frame_3
- E2eReplay/ReplayE2eTest.Runs/38: replay_truncated_frame_4
- E2eReplay/ReplayE2eTest.Runs/39: replay_truncated_frame_5

## Log Excerpt
```
Value of: result.has_error
  Actual: false
Expected: true
Expected: (result.error.message.find("Failed to read replay message body")) != (std::string::npos)
```

## Impact
Replay system does not properly detect or report truncated frame bodies.

## Source
See .github/logs/full_save_restore_run_*.log for details.