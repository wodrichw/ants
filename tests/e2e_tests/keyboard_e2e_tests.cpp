#include <gtest/gtest.h>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>

#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "app/engine_state.hpp"
#include "e2e_helpers.hpp"
#include "test_renderer.hpp"
#include "ui/colors.hpp"

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

    static void char_event(EngineState& state, uint32_t key) {
        state.dispatch_char_event(key);
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

void press_chord(EngineState& state, SDL_Keycode chord_key,
                 SDL_Keycode sym) {
    KeyboardE2eTestAccess::key_down(state, chord_key);
    KeyboardE2eTestAccess::key_down(state, sym);
    KeyboardE2eTestAccess::key_up(state, sym);
    KeyboardE2eTestAccess::key_up(state, chord_key);
}

void send_char(EngineState& state, char ch) {
    KeyboardE2eTestAccess::char_event(state, static_cast<uint32_t>(ch));
}

void reset_editor(TextEditor& editor) {
    editor.reset();
    editor.go_to_text_y();
    editor.move_to_start_line();
}

std::vector<E2eCase> build_keyboard_cases() {
    std::vector<E2eCase> cases;

    auto add_case = [&](const std::string& name, std::function<void()> run) {
        cases.push_back(E2eCase{name, std::move(run)});
    };

    // Root listeners
    add_case("keyboard_root_toggle_editor", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        EXPECT_TRUE(state.state.is_primary());
        press_key(state, SDLK_SLASH);
        EXPECT_TRUE(state.state.is_editor());
        press_key(state, SDLK_SLASH);
        EXPECT_TRUE(state.state.is_primary());
    });

    add_case("keyboard_editor_opens_blank", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        EXPECT_TRUE(state.state.is_primary());
        press_key(state, SDLK_SLASH);
        EXPECT_TRUE(state.state.is_editor());

        const TextEditor& editor = state.editor_mode.get_editor();
        ASSERT_FALSE(editor.lines.empty());
        EXPECT_EQ(editor.lines.size(), 1u);
        EXPECT_EQ(editor.lines[0], "");
        EXPECT_EQ(editor.get_cursor_x(), 0u);
        EXPECT_EQ(editor.get_cursor_y(), 0u);
    });

    add_case("keyboard_root_autosave", []() {
        KeyboardWorld world(true);
        e2e::clear_save_file();
        press_key(world.state, SDLK_BACKSLASH);
        std::ifstream in(e2e::save_path(), std::ios::binary);
        EXPECT_TRUE(in.good());
    });

    // Primary mode movement listeners
    struct MoveCase {
        const char* name;
        SDL_Keycode key;
        long dx;
        long dy;
    };

    const std::vector<MoveCase> move_cases = {
        {"keyboard_primary_move_left", SDLK_LEFT, -1, 0},
        {"keyboard_primary_move_right", SDLK_RIGHT, 1, 0},
        {"keyboard_primary_move_up", SDLK_UP, 0, -1},
        {"keyboard_primary_move_down", SDLK_DOWN, 0, 1},
        {"keyboard_primary_move_h", SDLK_h, -1, 0},
        {"keyboard_primary_move_l", SDLK_l, 1, 0},
        {"keyboard_primary_move_k", SDLK_k, 0, -1},
        {"keyboard_primary_move_j", SDLK_j, 0, 1},
    };

    for(const auto& test : move_cases) {
        add_case(test.name, [test]() {
            KeyboardWorld world(true);
            auto& state = world.state;
            auto& player = state.entity_manager.player.get_data();
            auto& map = state.map_world.current_level().map;
            if(map.is_wall(player.x + test.dx, player.y + test.dy)) {
                GTEST_SKIP() << "Adjacent tile is wall";
            }
            const long old_x = player.x;
            const long old_y = player.y;
            press_key(state, test.key);
            EXPECT_EQ(player.x, old_x + test.dx);
            EXPECT_EQ(player.y, old_y + test.dy);
        });
    }

    add_case("keyboard_primary_background_updates_after_nursery", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        auto& player = state.entity_manager.player.get_data();
        auto& map = state.map_world.current_level().map;

        Building* start_building = map.get_building(state.entity_manager.player);
        if(start_building == nullptr || start_building->get_type() != NURSERY) {
            GTEST_SKIP() << "Player is not in nursery at start";
        }

        if(map.is_wall(player.x - 1, player.y) ||
           map.is_wall(player.x - 2, player.y)) {
            GTEST_SKIP() << "Left tiles are walls";
        }

        state.update();
        state.render();

        EXPECT_TRUE(world.renderer.has_tile(player.x, player.y));
        EXPECT_EQ(world.renderer.get_tile_bg(player.x, player.y), color::blue);

        const long start_x = player.x;
        const long start_y = player.y;

        press_key(state, SDLK_LEFT);
        state.update();
        state.render();

        press_key(state, SDLK_LEFT);
        state.update();
        state.render();

        EXPECT_EQ(player.x, start_x - 2);
        EXPECT_EQ(player.y, start_y);
        EXPECT_TRUE(world.renderer.has_tile(player.x, player.y));
        EXPECT_EQ(world.renderer.get_tile_bg(player.x, player.y), color::grey);
    });

    // Create ant listener
    add_case("keyboard_primary_create_ant", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        state.action_add_program_lines({"NOP"});

        auto* building = state.map_world.current_level().map.get_building(
            state.entity_manager.player);
        if(building == nullptr) {
            GTEST_SKIP() << "No building at player location";
        }

        const auto before = state.entity_manager.num_workers();
        press_key(state, SDLK_a);
        const auto after = state.entity_manager.num_workers();
        EXPECT_EQ(after, before + 1);
    });

    // Level change listeners
    add_case("keyboard_primary_change_depth", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        const auto start_depth = state.map_world.current_depth;
        press_key(state, SDLK_q);
        EXPECT_EQ(state.map_world.current_depth, start_depth + 1);
        press_key(state, SDLK_e);
        EXPECT_EQ(state.map_world.current_depth, start_depth);
    });

    // Dig chord listeners
    struct DigCase {
        const char* name;
        SDL_Keycode key;
        long dx;
        long dy;
    };

    const std::vector<DigCase> dig_cases = {
        {"keyboard_primary_dig_left", SDLK_LEFT, -1, 0},
        {"keyboard_primary_dig_right", SDLK_RIGHT, 1, 0},
        {"keyboard_primary_dig_up", SDLK_UP, 0, -1},
        {"keyboard_primary_dig_down", SDLK_DOWN, 0, 1},
        {"keyboard_primary_dig_h", SDLK_h, -1, 0},
        {"keyboard_primary_dig_l", SDLK_l, 1, 0},
        {"keyboard_primary_dig_k", SDLK_k, 0, -1},
        {"keyboard_primary_dig_j", SDLK_j, 0, 1},
    };

    for(const auto& test : dig_cases) {
        add_case(test.name, [test]() {
            KeyboardWorld world(true);
            auto& state = world.state;
            auto& player = state.entity_manager.player.get_data();
            auto& map = state.map_world.current_level().map;
            if(!map.is_wall(player.x + test.dx, player.y + test.dy)) {
                GTEST_SKIP() << "Adjacent tile is not a wall";
            }
            press_chord(state, SDLK_d, test.key);
            EXPECT_FALSE(map.is_wall(player.x + test.dx, player.y + test.dy));
        });
    }

    // Reload listener
    add_case("keyboard_primary_reload", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        EXPECT_FALSE(state.is_reload_game);
        press_key(state, SDLK_r);
        EXPECT_TRUE(state.is_reload_game);
    });

    // Tile rendering listeners
    add_case("keyboard_primary_render_default", []() {
        KeyboardWorld world(true);
        press_key(world.state, SDLK_0);
        EXPECT_EQ(world.renderer.get_render_mode(),
                  TestRenderer::RenderMode::DEFAULT);
    });

    for(int i = 1; i <= 8; ++i) {
        const std::string name =
            "keyboard_primary_render_scent_" + std::to_string(i);
        const int expected_idx = i - 1;
        add_case(name, [expected_idx, i]() {
            KeyboardWorld world(true);
            press_key(world.state, SDLK_0 + i);
            EXPECT_EQ(world.renderer.get_render_mode(),
                      TestRenderer::RenderMode::SCENT);
            EXPECT_EQ(world.renderer.get_scent_index(),
                      static_cast<ulong>(expected_idx));
        });
    }

    // Editor listeners
    add_case("keyboard_editor_new_line", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        const auto before = editor.lines.size();
        press_key(state, SDLK_RETURN);
        EXPECT_EQ(editor.lines.size(), before + 1);
    });

    add_case("keyboard_editor_backspace", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        press_key(state, SDLK_a);
        EXPECT_EQ(editor.lines[0].size(), 1u);
        press_key(state, SDLK_BACKSPACE);
        EXPECT_EQ(editor.lines[0].size(), 0u);
    });

    add_case("keyboard_editor_close_after_typing", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        press_key(state, SDLK_s);
        press_key(state, SDLK_d);
        press_key(state, SDLK_a);
        press_key(state, SDLK_f);
        press_key(state, SDLK_SLASH);
        EXPECT_TRUE(state.state.is_primary());
    });

    add_case("keyboard_editor_move_left", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        editor.insert('A');
        editor.insert('B');
        editor.move_to_end_line();
        press_key(state, SDLK_LEFT);
        EXPECT_EQ(editor.get_cursor_x(), 1);
    });

    add_case("keyboard_editor_move_right", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        editor.insert('A');
        editor.insert('B');
        editor.move_to_start_line();
        press_key(state, SDLK_RIGHT);
        EXPECT_EQ(editor.get_cursor_x(), 1);
    });

    add_case("keyboard_editor_move_up", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        editor.insert('A');
        editor.new_line();
        editor.insert('B');
        press_key(state, SDLK_UP);
        EXPECT_EQ(editor.get_cursor_y(), 0);
    });

    add_case("keyboard_editor_move_down", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        editor.insert('A');
        editor.new_line();
        editor.insert('B');
        editor.move_up();
        editor.move_to_start_line();
        press_key(state, SDLK_DOWN);
        EXPECT_EQ(editor.get_cursor_y(), 1);
    });

    add_case("keyboard_editor_wrap_right", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        editor.insert('A');
        editor.new_line();
        editor.insert('B');
        editor.move_up();
        editor.move_to_end_line();
        press_key(state, SDLK_RIGHT);
        EXPECT_EQ(editor.get_cursor_y(), 1);
        EXPECT_EQ(editor.get_cursor_x(), 0);
    });

    add_case("keyboard_editor_wrap_left", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        editor.insert('A');
        editor.new_line();
        editor.insert('B');
        editor.move_to_start_line();
        press_key(state, SDLK_LEFT);
        EXPECT_EQ(editor.get_cursor_y(), 0);
        EXPECT_EQ(editor.get_cursor_x(), 1);
    });

    struct CharKeyCase {
        const char* label;
        SDL_Keycode key;
    };

    const std::vector<CharKeyCase> char_keys = {
        {"a", SDLK_a}, {"b", SDLK_b}, {"c", SDLK_c}, {"d", SDLK_d},
        {"e", SDLK_e}, {"f", SDLK_f}, {"g", SDLK_g}, {"h", SDLK_h},
        {"i", SDLK_i}, {"j", SDLK_j}, {"k", SDLK_k}, {"l", SDLK_l},
        {"m", SDLK_m}, {"n", SDLK_n}, {"o", SDLK_o}, {"p", SDLK_p},
        {"q", SDLK_q}, {"r", SDLK_r}, {"s", SDLK_s}, {"t", SDLK_t},
        {"u", SDLK_u}, {"v", SDLK_v}, {"w", SDLK_w}, {"x", SDLK_x},
        {"y", SDLK_y}, {"z", SDLK_z},
        {"0", SDLK_0}, {"1", SDLK_1}, {"2", SDLK_2}, {"3", SDLK_3},
        {"4", SDLK_4}, {"5", SDLK_5}, {"6", SDLK_6}, {"7", SDLK_7},
        {"8", SDLK_8}, {"9", SDLK_9},
        {"comma", SDLK_COMMA}, {"space", SDLK_SPACE},
        {"slash", SDLK_SLASH}, {"minus", SDLK_MINUS},
    };

    for(const auto& item : char_keys) {
        const std::string name = "keyboard_editor_char_" +
                                 std::string(item.label);
        add_case(name, [item]() {
            KeyboardWorld world(true);
            auto& state = world.state;
            press_key(state, SDLK_SLASH);
            auto& editor = state.editor_mode.get_editor();
            reset_editor(editor);
            if(item.key == SDLK_SLASH) {
                send_char(state, '/');
            } else {
                press_key(state, item.key);
            }
            ASSERT_FALSE(editor.lines.empty());
            EXPECT_EQ(editor.lines[0].size(), 1u);
        });
    }

    add_case("keyboard_editor_char_hash", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        press_key(state, SDLK_3, KMOD_LSHIFT);
        EXPECT_EQ(editor.lines[0], "#");
    });

    add_case("keyboard_editor_char_colon", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        press_key(state, SDLK_SEMICOLON, KMOD_LSHIFT);
        EXPECT_EQ(editor.lines[0], ":");
    });

    add_case("keyboard_editor_char_rejected", []() {
        KeyboardWorld world(true);
        auto& state = world.state;
        press_key(state, SDLK_SLASH);
        auto& editor = state.editor_mode.get_editor();
        reset_editor(editor);
        send_char(state, '!');
        EXPECT_EQ(editor.lines[0].size(), 0u);
    });

    return cases;
}

}  // namespace

class KeyboardE2eTest : public ::testing::TestWithParam<E2eCase> {};

TEST_P(KeyboardE2eTest, Runs) { GetParam().run(); }

INSTANTIATE_TEST_SUITE_P(E2eKeyboard,
                         KeyboardE2eTest,
                         ::testing::ValuesIn(build_keyboard_cases()));
