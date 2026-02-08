#include <gtest/gtest.h>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>

#include <cstdio>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "app/clock_speed.hpp"
#include "app/engine_state.hpp"
#include "e2e_helpers.hpp"
#include "test_renderer.hpp"
#include "ui/event_system.hpp"

class KeyboardE2eTestAccess {
   public:
    static void key_down(EngineState& state, SDL_Keycode sym, Uint16 mod = 0) {
        SDL_Keysym key = {};
        key.sym = sym;
        key.mod = mod;
        KeyboardEvent keyboard_event = {};
        CharKeyboardEvent char_keyboard_event = {};
        state.dispatch_key_down(key, keyboard_event, char_keyboard_event);
    }

    static void key_up(EngineState& state, SDL_Keycode sym, Uint16 mod = 0) {
        SDL_Keysym key = {};
        key.sym = sym;
        key.mod = mod;
        state.dispatch_key_up(key);
    }

    static void mouse_event(EngineState& state, const MouseEvent& event) {
        state.dispatch_mouse_event(event);
    }
};

namespace {

using e2e::E2eCase;

struct KeyboardWorld {
    ProjectArguments config;
    TestRenderer renderer;
    EngineState state;

    explicit KeyboardWorld(bool walls_enabled = true)
        : config("", e2e::save_path(), "", "", false, false, walls_enabled),
          renderer(),
          state(config, &renderer) {}
};

void press_key(EngineState& state, SDL_Keycode sym, Uint16 mod = 0) {
    KeyboardE2eTestAccess::key_down(state, sym, mod);
    KeyboardE2eTestAccess::key_up(state, sym, mod);
}

void press_mouse(EngineState& state, long x, long y) {
    MouseEvent event;
    event.x = x;
    event.y = y;
    event.type = LEFT_MOUSE_EVENT;
    KeyboardE2eTestAccess::mouse_event(state, event);
}

void open_sidebar(EngineState& state) {
    if(!state.box_manager.is_sidebar_expanded()) {
        press_key(state, SDLK_m);
    }
}

std::vector<E2eCase> build_sidebar_cases() {
    std::vector<E2eCase> cases;

    auto add_case = [&](const std::string& name, std::function<void()> run) {
        cases.push_back(E2eCase{name, std::move(run)});
    };

    add_case("sidebar_toggle_with_m", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        EXPECT_FALSE(state.box_manager.is_sidebar_expanded());
        press_key(state, SDLK_m);
        EXPECT_TRUE(state.box_manager.is_sidebar_expanded());
        press_key(state, SDLK_m);
        EXPECT_FALSE(state.box_manager.is_sidebar_expanded());
    });

    add_case("sidebar_blocks_movement_when_expanded", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        auto& player = state.entity_manager.player.get_data();
        const long start_x = player.x;
        const long start_y = player.y;

        open_sidebar(state);
        press_key(state, SDLK_UP);

        EXPECT_EQ(player.x, start_x);
        EXPECT_EQ(player.y, start_y);
    });

    add_case("sidebar_navigation_updates_selection", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        open_sidebar(state);

        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_DOWN);
        EXPECT_EQ(state.sidebar_menu.selected_index(), 2u);

        press_key(state, SDLK_UP);
        EXPECT_EQ(state.sidebar_menu.selected_index(), 1u);
    });

    add_case("sidebar_save_action_writes_file", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        e2e::clear_save_file();
        open_sidebar(state);

        press_key(state, SDLK_RETURN);

        std::ifstream in(e2e::save_path(), std::ios::binary);
        EXPECT_TRUE(in.good());
        e2e::clear_save_file();
    });

    add_case("sidebar_restore_sets_reload", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        open_sidebar(state);

        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_RETURN);
        EXPECT_TRUE(state.is_reload_game);
    });

    add_case("sidebar_text_editor_toggle", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        open_sidebar(state);

        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_RETURN);

        EXPECT_TRUE(state.state.is_editor());
    });

    add_case("sidebar_submenu_open_close", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        open_sidebar(state);

        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_RETURN);
        EXPECT_TRUE(state.sidebar_menu.is_submenu_open());

        press_key(state, SDLK_LEFT);
        EXPECT_FALSE(state.sidebar_menu.is_submenu_open());
    });

    add_case("sidebar_submenu_action", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        open_sidebar(state);

        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_RETURN);
        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_RETURN);

        EXPECT_FALSE(state.sidebar_menu.take_pending_action().has_value());
    });

    add_case("sidebar_mouse_toggle_hits_button", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        auto& renderer = world.renderer;

        renderer.pixel_tile_x = globals::COLS - 1;
        renderer.pixel_tile_y = 0;
        press_mouse(state, 0, 0);
        EXPECT_TRUE(state.box_manager.is_sidebar_expanded());
    });

    add_case("sidebar_mouse_toggle_miss", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        auto& renderer = world.renderer;

        renderer.pixel_tile_x = 0;
        renderer.pixel_tile_y = 0;
        press_mouse(state, 0, 0);
        EXPECT_FALSE(state.box_manager.is_sidebar_expanded());
    });

    add_case("sidebar_render_flags_when_collapsed", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        auto& renderer = world.renderer;

        renderer.reset_render_flags();
        state.render();

        EXPECT_FALSE(renderer.was_sidebar_rendered());
        EXPECT_TRUE(renderer.was_toggle_button_rendered());
        EXPECT_FALSE(renderer.get_toggle_button_expanded());
    });

    add_case("sidebar_background_is_grey", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        auto& renderer = world.renderer;
        open_sidebar(state);

        state.render();

        ASSERT_TRUE(renderer.has_sidebar_tiles());
        const long w = renderer.get_sidebar_width();
        const long h = renderer.get_sidebar_height();
        ASSERT_GT(w, 0);
        ASSERT_GT(h, 0);

        for(long y = 0; y < h; ++y) {
            for(long x = 0; x < w; ++x) {
                EXPECT_EQ(renderer.get_sidebar_bg(x, y), color::dark_grey)
                    << "Sidebar background mismatch at (" << x << ", " << y
                    << ")";
            }
        }
    });

    add_case("sidebar_box_fits_inside_region", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        auto& renderer = world.renderer;
        open_sidebar(state);

        state.render();

        ASSERT_TRUE(renderer.has_sidebar_tiles());
        const long w = renderer.get_sidebar_width();
        const long h = renderer.get_sidebar_height();
        ASSERT_GE(w, 4);
        ASSERT_GE(h, 4);

        EXPECT_EQ(renderer.get_sidebar_ch(0, 0), ' ');
        EXPECT_EQ(renderer.get_sidebar_ch(1, 1), '+');
        EXPECT_EQ(renderer.get_sidebar_ch(w - 2, 1), '+');
        EXPECT_EQ(renderer.get_sidebar_ch(1, h - 2), '+');
        EXPECT_EQ(renderer.get_sidebar_ch(w - 2, h - 2), '+');
        EXPECT_EQ(renderer.get_sidebar_ch(w - 1, 0), ' ');
        EXPECT_EQ(renderer.get_sidebar_ch(0, h - 1), ' ');
    });

    add_case("sidebar_clock_pause_sets_paused", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        open_sidebar(state);

        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_RETURN);
        press_key(state, SDLK_RETURN);

        EXPECT_EQ(state.clock_speed, ClockSpeed::PAUSED);
    });

    add_case("sidebar_clock_play_sets_normal", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        open_sidebar(state);

        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_RETURN);
        press_key(state, SDLK_RETURN);
        ASSERT_EQ(state.clock_speed, ClockSpeed::PAUSED);

        press_key(state, SDLK_RETURN);
        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_RETURN);

        EXPECT_EQ(state.clock_speed, ClockSpeed::NORMAL);
    });

    add_case("sidebar_clock_fast_forward_sets_fast", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        open_sidebar(state);

        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_RETURN);
        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_DOWN);
        press_key(state, SDLK_RETURN);

        EXPECT_EQ(state.clock_speed, ClockSpeed::FAST);
    });

    add_case("sidebar_clock_speed_default_is_normal", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        EXPECT_EQ(state.clock_speed, ClockSpeed::NORMAL);
    });

    return cases;
}

class SidebarE2eTest : public ::testing::TestWithParam<E2eCase> {};

TEST_P(SidebarE2eTest, Runs) { GetParam().run(); }

INSTANTIATE_TEST_SUITE_P(E2eSidebar,
                         SidebarE2eTest,
                         ::testing::ValuesIn(build_sidebar_cases()));

}  // namespace