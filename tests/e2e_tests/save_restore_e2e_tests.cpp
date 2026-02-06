#include "app/engine.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "app/globals.hpp"
#include "e2e_helpers.hpp"
#include "entity/ant.hpp"
#include "ui/ui_handlers.hpp"

namespace {

using e2e::E2eCase;

struct SaveRestoreScenario {
    std::string name;
    cpu_word_size reg_a;
    cpu_word_size reg_b;
    bool zero_flag;
    bool instr_failed_flag;
    bool dir_flag1;
    bool dir_flag2;
    bool is_move_flag;
    bool is_dig_flag;
    unsigned short instr_ptr;
    ulong instr_trigger;
    std::vector<std::string> program_lines;
    ulong move_index;
    ulong dig_index;
};

struct ScenarioResult {
    e2e::WorldSnapshot snapshot;
    std::pair<long, long> control_wall_tile;
};

std::string make_temp_path(const std::string& prefix) {
    auto ts = std::chrono::steady_clock::now().time_since_epoch().count();
    return (std::filesystem::temp_directory_path() /
            (prefix + std::to_string(ts)))
        .string();
}

std::vector<SaveRestoreScenario> build_scenarios() {
    return {
        {"scenario_1",
         1,
         2,
         false,
         false,
         false,
         false,
         false,
         false,
         0,
         0,
         {"NOP"},
         0,
         0},
        {"scenario_2",
         10,
         20,
         true,
         true,
         true,
         false,
         true,
         false,
         3,
         1,
         {"DIG"},
         1,
         1},
        {"scenario_3",
         7,
         4,
         false,
         true,
         false,
         true,
         false,
         true,
         5,
         2,
         {"LT"},
         2,
         2},
        {"scenario_4",
         123,
         456,
         true,
         false,
         true,
         true,
         false,
         false,
         1,
         4,
         {"RT"},
         3,
         3},
        {"scenario_5",
         0,
         999,
         false,
         false,
         false,
         true,
         true,
         false,
         2,
         0,
         {"NOP", "NOP"},
         4,
         4},
        {"scenario_6",
         42,
         24,
         true,
         true,
         true,
         false,
         false,
         true,
         7,
         3,
         {"DIG", "NOP"},
         0,
         1},
        {"scenario_7",
         11,
         12,
         false,
         true,
         false,
         true,
         true,
         false,
         8,
         6,
         {"LT", "RT"},
         1,
         2},
        {"scenario_8",
         255,
         1,
         true,
         false,
         true,
         false,
         false,
         true,
         9,
         0,
         {"NOP", "LT", "RT"},
         2,
         3},
        {"scenario_9",
         5,
         6,
         false,
         false,
         false,
         false,
         true,
         true,
         4,
         5,
         {"RT", "DIG"},
         3,
         4},
        {"scenario_10",
         99,
         100,
         true,
         true,
         true,
         true,
         false,
         false,
         6,
         7,
         {"LT", "DIG"},
         4,
         0},
    };
}

ScenarioResult setup_scenario(EngineState& state,
                              const SaveRestoreScenario& scenario) {
    ScenarioResult result{};
    auto& player = state.entity_manager.player.get_data();
    auto& map = state.map_world.current_level().map;

    const auto floor_offsets =
        e2e::collect_offsets(map, player.x, player.y, false, 5);
    if(floor_offsets.size() < 5u) {
        ADD_FAILURE() << "Not enough floor offsets.";
        return result;
    }
    const auto [move_dx, move_dy] =
        floor_offsets[scenario.move_index % floor_offsets.size()];
    state.action_move_player(move_dx, move_dy);

    const auto wall_offsets =
        e2e::collect_offsets(map, player.x, player.y, true, 5);
    if(wall_offsets.size() < 5u) {
        ADD_FAILURE() << "Not enough wall offsets.";
        return result;
    }

    std::vector<std::pair<long, long>> tracked_tiles;
    tracked_tiles.reserve(5);
    for(ulong i = 0; i < 3; ++i) {
        const auto [dx, dy] =
            wall_offsets[static_cast<decltype(wall_offsets)::size_type>(
                (scenario.dig_index + i) %
                static_cast<ulong>(wall_offsets.size()))];
        state.action_dig(dx, dy);
        tracked_tiles.emplace_back(player.x + dx, player.y + dy);
    }

    const auto [control_dx, control_dy] =
        wall_offsets[static_cast<decltype(wall_offsets)::size_type>(
            (scenario.dig_index + 3) %
            static_cast<ulong>(wall_offsets.size()))];
    std::pair<long, long> control_wall_tile{player.x + control_dx,
                                            player.y + control_dy};
    tracked_tiles.push_back(control_wall_tile);
    tracked_tiles.emplace_back(player.x, player.y);

    state.action_add_program_lines(scenario.program_lines);
    if(!state.software_manager.has_code()) {
        ADD_FAILURE() << "Program code not available.";
        return result;
    }

    auto* building = map.get_building(state.entity_manager.player);
    if(building == nullptr) {
        ADD_FAILURE() << "Expected player building.";
        return result;
    }

    const auto before = state.entity_manager.num_workers();
    state.action_create_ant();
    if(state.entity_manager.num_workers() != before + 1) {
        ADD_FAILURE() << "Worker count did not increment.";
        return result;
    }

    Worker* worker = state.map_world.current_level().workers.back();
    worker->cpu.registers[0] = scenario.reg_a;
    worker->cpu.registers[1] = scenario.reg_b;
    worker->cpu.zero_flag = scenario.zero_flag;
    worker->cpu.instr_failed_flag = scenario.instr_failed_flag;
    worker->cpu.dir_flag1 = scenario.dir_flag1;
    worker->cpu.dir_flag2 = scenario.dir_flag2;
    worker->cpu.is_move_flag = scenario.is_move_flag;
    worker->cpu.is_dig_flag = scenario.is_dig_flag;
    worker->cpu.instr_ptr_register = scenario.instr_ptr;
    worker->program_executor.instr_trigger = scenario.instr_trigger;

    result.control_wall_tile = control_wall_tile;
    result.snapshot = e2e::capture_world_snapshot(state, tracked_tiles);
    return result;
}

void mutate_world(EngineState& state,
                  const std::pair<long, long>& control_wall_tile) {
    auto& player = state.entity_manager.player.get_data();
    const long dx = control_wall_tile.first - player.x;
    const long dy = control_wall_tile.second - player.y;
    state.action_dig(dx, dy);

    auto& map = state.map_world.current_level().map;
    const auto floor_offsets =
        e2e::collect_offsets(map, player.x, player.y, false, 1);
    if(!floor_offsets.empty()) {
        state.action_move_player(floor_offsets.front().first,
                                 floor_offsets.front().second);
    }

    if(!state.map_world.current_level().workers.empty()) {
        Worker* worker = state.map_world.current_level().workers.front();
        worker->cpu.registers[0] += 3;
        worker->cpu.registers[1] += 5;
        worker->cpu.zero_flag = !worker->cpu.zero_flag;
        worker->cpu.instr_failed_flag = !worker->cpu.instr_failed_flag;
        worker->cpu.dir_flag1 = !worker->cpu.dir_flag1;
        worker->cpu.dir_flag2 = !worker->cpu.dir_flag2;
        worker->cpu.instr_ptr_register += 1;
        worker->program_executor.instr_trigger += 1;
    }

    state.action_add_program_lines({"SWF"});
}

void perform_reload(Engine& engine) {
    engine.reload_state_for_tests();
}

std::vector<E2eCase> build_save_restore_cases() {
    std::vector<E2eCase> cases;

    auto add_case = [&](const std::string& name,
                        std::function<void()> run) {
        cases.push_back(E2eCase{name, std::move(run)});
    };

    const auto scenarios = build_scenarios();

    for(ulong i = 0; i < static_cast<ulong>(scenarios.size()); ++i) {
        const auto scenario = scenarios[i];
        const std::string name =
            "save_restore_startup_" + std::to_string(i + 1);
        add_case(name, [scenario]() {
            e2e::clear_save_file();
            ProjectArguments config("", e2e::save_path(), "", "", false,
                                    false, true);

            Engine engine(config);
            ASSERT_NE(engine.get_state(), nullptr);

            const auto result = setup_scenario(*engine.get_state(), scenario);
            e2e::trigger_save(*engine.get_state(), e2e::save_path());

            Engine restored(config);
            ASSERT_NE(restored.get_state(), nullptr);
            e2e::assert_world_state(*restored.get_state(), result.snapshot);
        });
    }

    for(ulong i = 0; i < static_cast<ulong>(scenarios.size()); ++i) {
        const auto scenario = scenarios[i];
        const std::string name =
            "save_restore_reload_" + std::to_string(i + 1);
        add_case(name, [scenario]() {
            e2e::clear_save_file();
            ProjectArguments config("", e2e::save_path(), "", "", false,
                                    false, true);

            Engine engine(config);
            ASSERT_NE(engine.get_state(), nullptr);

            const auto result = setup_scenario(*engine.get_state(), scenario);
            e2e::trigger_save(*engine.get_state(), e2e::save_path());

            mutate_world(*engine.get_state(), result.control_wall_tile);

            ReloadGameHandler reload_handler(
                engine.get_state()->is_reload_game);
            KeyboardEvent reload_event;
            reload_handler(reload_event);
            perform_reload(engine);

            ASSERT_NE(engine.get_state(), nullptr);
            e2e::assert_world_state(*engine.get_state(), result.snapshot);
        });
    }

    add_case("save_restore_empty_file_1", []() {
        const std::string path = make_temp_path("e2e_empty_save_");
        {
            std::ofstream out(path, std::ios::binary);
        }

        ProjectArguments config("", path, "", "", false, false, true);
        Engine engine(config);
        ASSERT_NE(engine.get_state(), nullptr);

        std::filesystem::remove(path);
    });

    return cases;
}

}  // namespace

class SaveRestoreE2eTest : public ::testing::TestWithParam<E2eCase> {};

TEST_P(SaveRestoreE2eTest, Runs) { GetParam().run(); }

INSTANTIATE_TEST_SUITE_P(E2eSaveRestore,
                         SaveRestoreE2eTest,
                         ::testing::ValuesIn(build_save_restore_cases()));
