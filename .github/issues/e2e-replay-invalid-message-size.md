# E2E Replay: Invalid Message Size Test Failures

## Summary
Several E2E replay tests failed due to not detecting invalid replay message sizes as errors.

## Failing Tests
- E2eReplay/ReplayE2eTest.Runs/30: replay_invalid_message_size_1
- E2eReplay/ReplayE2eTest.Runs/31: replay_invalid_message_size_2
- E2eReplay/ReplayE2eTest.Runs/32: replay_invalid_message_size_3
- E2eReplay/ReplayE2eTest.Runs/33: replay_invalid_message_size_4
- E2eReplay/ReplayE2eTest.Runs/34: replay_invalid_message_size_5

## Log Excerpt
```
Value of: result.has_error
  Actual: false
Expected: true
Expected: (result.error.message.find("Invalid replay message size")) != (std::string::npos)
```

## Impact
Replay system does not properly detect or report invalid message sizes.

## Source
See .github/logs/full_save_restore_run_*.log for details.