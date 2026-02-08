#include <gtest/gtest.h>

#include "app/clock_speed.hpp"
#include "app/engine_state.hpp"
#include "ui/render.hpp"

TEST(ClockSpeedTest, DefaultClockSpeedIsNormal) {
    ProjectArguments config("", "", "", "", false, false, true);
    NoneRenderer renderer;
    EngineState state(config, &renderer);
    EXPECT_EQ(state.clock_speed, ClockSpeed::NORMAL);
}

TEST(ClockSpeedTest, HandleSidebarActionSetsPaused) {
    ProjectArguments config("", "", "", "", false, false, true);
    NoneRenderer renderer;
    EngineState state(config, &renderer);
    state.handle_sidebar_action(SidebarMenuAction::CLOCK_PAUSE);
    EXPECT_EQ(state.clock_speed, ClockSpeed::PAUSED);
}

TEST(ClockSpeedTest, HandleSidebarActionSetsNormal) {
    ProjectArguments config("", "", "", "", false, false, true);
    NoneRenderer renderer;
    EngineState state(config, &renderer);
    state.handle_sidebar_action(SidebarMenuAction::CLOCK_PAUSE);
    state.handle_sidebar_action(SidebarMenuAction::CLOCK_PLAY);
    EXPECT_EQ(state.clock_speed, ClockSpeed::NORMAL);
}

TEST(ClockSpeedTest, HandleSidebarActionSetsFast) {
    ProjectArguments config("", "", "", "", false, false, true);
    NoneRenderer renderer;
    EngineState state(config, &renderer);
    state.handle_sidebar_action(SidebarMenuAction::CLOCK_FAST_FORWARD);
    EXPECT_EQ(state.clock_speed, ClockSpeed::FAST);
}

TEST(ClockSpeedTest, CyclesThroughAllSpeeds) {
    ProjectArguments config("", "", "", "", false, false, true);
    NoneRenderer renderer;
    EngineState state(config, &renderer);

    state.handle_sidebar_action(SidebarMenuAction::CLOCK_FAST_FORWARD);
    EXPECT_EQ(state.clock_speed, ClockSpeed::FAST);

    state.handle_sidebar_action(SidebarMenuAction::CLOCK_PAUSE);
    EXPECT_EQ(state.clock_speed, ClockSpeed::PAUSED);

    state.handle_sidebar_action(SidebarMenuAction::CLOCK_PLAY);
    EXPECT_EQ(state.clock_speed, ClockSpeed::NORMAL);
}
