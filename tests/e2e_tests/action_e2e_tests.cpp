#include <gtest/gtest.h>

#include <functional>
#include <string>
#include <vector>

#include "e2e_helpers.hpp"

namespace {

using e2e::E2eCase;

std::vector<E2eCase> build_action_cases() {
    std::vector<E2eCase> cases;

    auto add_case = [&](const std::string& name,
                        std::function<void()> run) {
        cases.push_back(E2eCase{name, std::move(run)});
    };

    // Area 11: Move to floor tiles (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name = "action_move_floor_" + std::to_string(i + 1);
        const ulong index = static_cast<ulong>(i);
        add_case(name, [index]() {
            e2e::ActionWorld world(true);
            auto& player = world.state.entity_manager.player.get_data();
            auto& map = world.state.map_world.current_level().map;
            auto offsets =
                e2e::collect_offsets(map, player.x, player.y, false, 5);
            ASSERT_GE(offsets.size(), 5u);
            const auto [dx, dy] = offsets[static_cast<decltype(offsets)::size_type>(index)];
            const long old_x = player.x;
            const long old_y = player.y;
            world.state.action_move_player(dx, dy);
            EXPECT_EQ(player.x, old_x + dx);
            EXPECT_EQ(player.y, old_y + dy);
        });
    }

    // Area 12: Wall movement blocked (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name =
            "action_move_wall_blocked_" + std::to_string(i + 1);
        const ulong index = static_cast<ulong>(i);
        add_case(name, [index]() {
            e2e::ActionWorld world(true);
            auto& player = world.state.entity_manager.player.get_data();
            auto& map = world.state.map_world.current_level().map;
            auto offsets = e2e::collect_offsets(map, player.x, player.y, true, 5);
            ASSERT_GE(offsets.size(), 5u);
            const auto [dx, dy] = offsets[static_cast<decltype(offsets)::size_type>(index)];
            const long old_x = player.x;
            const long old_y = player.y;
            world.state.action_move_player(dx, dy);
            EXPECT_EQ(player.x, old_x);
            EXPECT_EQ(player.y, old_y);
        });
    }

    // Area 13: Wall movement allowed when walls disabled (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name =
            "action_move_wall_disabled_" + std::to_string(i + 1);
        const ulong index = static_cast<ulong>(i);
        add_case(name, [index]() {
            e2e::ActionWorld world(false);
            auto& player = world.state.entity_manager.player.get_data();
            auto& map = world.state.map_world.current_level().map;
            auto offsets = e2e::collect_offsets(map, player.x, player.y, true, 5);
            if(offsets.size() < 5u) {
                GTEST_SKIP() << "No wall offsets available when walls disabled";
            }
            const auto [dx, dy] = offsets[static_cast<decltype(offsets)::size_type>(index)];
            const long old_x = player.x;
            const long old_y = player.y;
            world.state.action_move_player(dx, dy);
            EXPECT_EQ(player.x, old_x + dx);
            EXPECT_EQ(player.y, old_y + dy);
        });
    }

    // Area 14: Digging removes wall (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name = "action_dig_wall_" + std::to_string(i + 1);
        const ulong index = static_cast<ulong>(i);
        add_case(name, [index]() {
            e2e::ActionWorld world(true);
            auto& player = world.state.entity_manager.player.get_data();
            auto& map = world.state.map_world.current_level().map;
            auto offsets = e2e::collect_offsets(map, player.x, player.y, true, 5);
            ASSERT_GE(offsets.size(), 5u);
            const auto [dx, dy] = offsets[static_cast<decltype(offsets)::size_type>(index)];
            ASSERT_TRUE(map.is_wall(player.x + dx, player.y + dy));
            world.state.action_dig(dx, dy);
            EXPECT_FALSE(map.is_wall(player.x + dx, player.y + dy));
        });
    }

    // Area 15: Digging floor does not add walls (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name = "action_dig_floor_" + std::to_string(i + 1);
        const ulong index = static_cast<ulong>(i);
        add_case(name, [index]() {
            e2e::ActionWorld world(true);
            auto& player = world.state.entity_manager.player.get_data();
            auto& map = world.state.map_world.current_level().map;
            auto offsets =
                e2e::collect_offsets(map, player.x, player.y, false, 5);
            ASSERT_GE(offsets.size(), 5u);
            const auto [dx, dy] = offsets[static_cast<decltype(offsets)::size_type>(index)];
            ASSERT_FALSE(map.is_wall(player.x + dx, player.y + dy));
            world.state.action_dig(dx, dy);
            EXPECT_FALSE(map.is_wall(player.x + dx, player.y + dy));
        });
    }

    // Area 16: Valid program lines are accepted (5 cases)
    {
        const std::vector<std::vector<std::string>> programs = {
            {"NOP"},
            {"MOVE DOWN"},
            {"INC A"},
            {"DEC B"},
            {"LDI A 1"},
        };
        for(ulong i = 0; i < static_cast<ulong>(programs.size()); ++i) {
            const std::string name =
                "action_program_valid_" + std::to_string(i + 1);
            const auto lines = programs[static_cast<decltype(programs)::size_type>(i)];
            add_case(name, [lines]() {
                e2e::ActionWorld world(true);
                world.state.action_add_program_lines(lines);
                EXPECT_TRUE(world.state.software_manager.has_code());
            });
        }
    }

    // Area 17: Invalid program lines clear code (5 cases)
    {
        const std::vector<std::vector<std::string>> programs = {
            {"INVALID"},
            {"MOVE"},
            {"ADD A"},
            {"LDI C 1"},
            {"JMP"},
        };
        for(ulong i = 0; i < static_cast<ulong>(programs.size()); ++i) {
            const std::string name =
                "action_program_invalid_" + std::to_string(i + 1);
            const auto lines = programs[static_cast<decltype(programs)::size_type>(i)];
            add_case(name, [lines]() {
                e2e::ActionWorld world(true);
                world.state.action_add_program_lines(lines);
                EXPECT_FALSE(world.state.software_manager.has_code());
            });
        }
    }

    // Area 18: Create ant without program (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name =
            "action_create_ant_no_program_" + std::to_string(i + 1);
        add_case(name, []() {
            e2e::ActionWorld world(true);
            const auto before = world.state.entity_manager.num_workers();
            world.state.action_create_ant();
            const auto after = world.state.entity_manager.num_workers();
            EXPECT_EQ(before, after);
        });
    }

    // Area 19: Create ant with program (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name =
            "action_create_ant_with_program_" + std::to_string(i + 1);
        add_case(name, []() {
            e2e::ActionWorld world(true);
            world.state.action_add_program_lines({"NOP"});

            auto* building = world.state.map_world.current_level().map.get_building(
                world.state.entity_manager.player);
            if(building == nullptr) {
                GTEST_SKIP() << "No building at player location";
            }

            const auto before = world.state.entity_manager.num_workers();
            world.state.action_create_ant();
            const auto after = world.state.entity_manager.num_workers();
            EXPECT_EQ(after, before + 1);
        });
    }

    // Area 20: Go down/up depth transitions (5 cases)
    for(int i = 1; i <= 5; ++i) {
        const std::string name =
            "action_depth_down_up_" + std::to_string(i);
        add_case(name, [i]() {
            e2e::ActionWorld world(true);
            const auto start_depth = world.state.map_world.current_depth;
            for(int step = 0; step < i; ++step) {
                world.state.action_go_down();
            }
            EXPECT_EQ(world.state.map_world.current_depth,
                      start_depth + static_cast<ulong>(i));
            for(int step = 0; step < i; ++step) {
                world.state.action_go_up();
            }
            EXPECT_EQ(world.state.map_world.current_depth, start_depth);
        });
    }

    return cases;
}

}  // namespace

class ActionE2eTest : public ::testing::TestWithParam<E2eCase> {};

TEST_P(ActionE2eTest, Runs) { GetParam().run(); }

INSTANTIATE_TEST_SUITE_P(E2eAction,
                         ActionE2eTest,
                         ::testing::ValuesIn(build_action_cases()));
