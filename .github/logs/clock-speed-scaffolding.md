# Clock Speed Scaffolding (Sidebar Menu)

Date: 2026-02-07

## Summary
The sidebar menu includes placeholder clock speed controls:
- Pause
- Play
- Fast Forward

These items are wired as no-op actions that log a placeholder message. They are ready for a future implementation that changes the engine tick rate.

## Current Stub Locations
- Sidebar menu definition: src/core/ui/sidebar_menu.cpp
- Action handling: src/core/app/engine_state.cpp (EngineState::handle_sidebar_action)

## Future Implementation Guidance
1. Add a clock speed field to EngineState, e.g.:
   - enum ClockSpeed { PAUSED, NORMAL, FAST }
   - or a numeric multiplier (0.0, 1.0, 2.0)
2. Update Engine::update() to apply the chosen tick rate.
   - Currently fixed at ~17ms via SDL timer usage.
3. Wire sidebar actions to set the clock speed field.
4. Optional: display the active clock speed in the sidebar UI.
