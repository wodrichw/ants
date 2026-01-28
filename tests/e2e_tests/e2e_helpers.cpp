#include "e2e_helpers.hpp"

#include <cstdio>
#include <fstream>
#include <ostream>

#include <gtest/gtest.h>

#include "ui/serializer_handler.hpp"

namespace e2e {

std::ostream& operator<<(std::ostream& os, const E2eCase& spec) {
    return os << spec.name;
}

std::string replay_dir() {
    return std::string(E2E_ASSET_DIR) + "/replays/generated";
}

std::string make_replay_path(const std::string& name) {
    return replay_dir() + "/" + name + ".replay";
}

ant_proto::ReplayHeader make_header(uint32_t version,
                                    uint32_t seed_x,
                                    uint32_t seed_y,
                                    bool walls_enabled,
                                    const std::string& map_path) {
    ant_proto::ReplayHeader header;
    header.set_version(version);
    header.set_created_unix_ms(0);
    header.set_build("e2e");

    auto* env = header.mutable_environment();
    env->set_region_seed_x(seed_x);
    env->set_region_seed_y(seed_y);
    env->set_is_walls_enabled(walls_enabled);
    env->set_map_path(map_path);

    return header;
}

ant_proto::ReplayEvent make_mouse_event(int64_t x, int64_t y, int32_t button) {
    ant_proto::ReplayEvent event;
    event.set_kind(ant_proto::REPLAY_EVENT_MOUSE_BUTTON_DOWN);
    auto* mouse = event.mutable_mouse();
    mouse->set_x(x);
    mouse->set_y(y);
    mouse->set_button(button);
    return event;
}

ant_proto::ReplayEvent make_key_event(ant_proto::ReplayEventKind kind,
                                      int32_t key_sym,
                                      int32_t key_mod) {
    ant_proto::ReplayEvent event;
    event.set_kind(kind);
    auto* key = event.mutable_key();
    key->set_key_sym(key_sym);
    key->set_key_mod(key_mod);
    return event;
}

ant_proto::ReplayEvent make_char_event(uint32_t key) {
    ant_proto::ReplayEvent event;
    event.set_kind(ant_proto::REPLAY_EVENT_CHAR);
    auto* ch = event.mutable_ch();
    ch->set_key(key);
    return event;
}

ant_proto::ReplayEvent make_bare_event(ant_proto::ReplayEventKind kind) {
    ant_proto::ReplayEvent event;
    event.set_kind(kind);
    return event;
}

ant_proto::ReplayFrame make_frame(
    uint64_t frame_index,
    const std::vector<ant_proto::ReplayEvent>& events) {
    ant_proto::ReplayFrame frame;
    frame.set_frame_index(frame_index);
    for(const auto& ev : events) {
        *frame.add_events() = ev;
    }
    return frame;
}

void write_replay(const std::string& path,
                  const ant_proto::ReplayHeader& header,
                  const std::vector<ant_proto::ReplayFrame>& frames) {
    Packer packer(path);
    packer << header;
    for(const auto& frame : frames) {
        packer << frame;
    }
}

void write_replay_with_bad_frame_size(const std::string& path,
                                      const ant_proto::ReplayHeader& header,
                                      int size) {
    Packer packer(path);
    packer << header;
    std::ofstream out(path, std::ios::binary | std::ios::app);
    out.write(reinterpret_cast<const char*>(&size), sizeof(int));
}

void write_replay_with_truncated_frame(const std::string& path,
                                       const ant_proto::ReplayHeader& header,
                                       int size,
                                       int bytes_written) {
    Packer packer(path);
    packer << header;
    std::ofstream out(path, std::ios::binary | std::ios::app);
    out.write(reinterpret_cast<const char*>(&size), sizeof(int));
    std::string data(static_cast<size_t>(bytes_written), '\0');
    out.write(data.data(), static_cast<std::streamsize>(data.size()));
}

ReplayRunResult run_replay(const std::string& path, size_t max_frames) {
    ProjectArguments config("", "", "", path, false, false, false);
    Engine engine(config);

    for(size_t i = 0; i < max_frames; ++i) {
        if(engine.replay_has_error()) break;
        if(engine.replay_done()) break;
        engine.update();
    }

    ReplayRunResult result;
    result.has_error = engine.replay_has_error();
    result.done = engine.replay_done();
    result.error = engine.replay_last_error();
    return result;
}

ActionWorld::ActionWorld(bool walls_enabled)
    : config("", "", "", "", false, false, walls_enabled),
      renderer(),
      state(config, &renderer) {}

std::vector<std::pair<long, long>> collect_offsets(Map& map,
                                                   long x,
                                                   long y,
                                                   bool want_wall,
                                                   size_t count) {
    std::vector<std::pair<long, long>> offsets;
    for(long radius = 1; radius <= 20; ++radius) {
        for(long dx = -radius; dx <= radius; ++dx) {
            for(long dy = -radius; dy <= radius; ++dy) {
                if(dx == 0 && dy == 0) continue;
                if(map.is_wall(x + dx, y + dy) != want_wall) continue;
                offsets.emplace_back(dx, dy);
                if(offsets.size() >= count) return offsets;
            }
        }
    }
    return offsets;
}

std::string save_dir() {
    return std::string(E2E_ASSET_DIR) + "/saves";
}

std::string save_path() { return save_dir() + "/e2e_save.bin"; }

void clear_save_file() {
    const auto path = save_path();
    std::remove(path.c_str());
}

void trigger_save(EngineState& state, const std::string& path) {
    AutoSaveTriggerHandler handler(state, path);
    KeyboardEvent event;
    handler(event);
}

WorldSnapshot capture_world_snapshot(
    EngineState& state,
    const std::vector<std::pair<long, long>>& map_tiles) {
    WorldSnapshot snapshot;
    auto& player = state.entity_manager.player.get_data();
    snapshot.player_x = player.x;
    snapshot.player_y = player.y;
    snapshot.player_depth = state.entity_manager.player_depth;
    snapshot.current_depth = state.map_world.current_depth;

    snapshot.worker_count = state.entity_manager.num_workers();
    snapshot.workers.reserve(snapshot.worker_count);
    for(auto& level : state.map_world.levels) {
        for(auto* worker : level.workers) {
            WorkerSnapshot worker_snapshot;
            const auto& data = worker->get_data();
            const auto& cpu = worker->cpu;
            worker_snapshot.x = data.x;
            worker_snapshot.y = data.y;
            worker_snapshot.reg_a = cpu.registers[0];
            worker_snapshot.reg_b = cpu.registers[1];
            worker_snapshot.zero_flag = cpu.zero_flag;
            worker_snapshot.instr_failed_flag = cpu.instr_failed_flag;
            worker_snapshot.dir_flag1 = cpu.dir_flag1;
            worker_snapshot.dir_flag2 = cpu.dir_flag2;
            worker_snapshot.is_move_flag = cpu.is_move_flag;
            worker_snapshot.is_dig_flag = cpu.is_dig_flag;
            worker_snapshot.instr_ptr = cpu.instr_ptr_register;
            worker_snapshot.instr_trigger = worker->program_executor.instr_trigger;
            snapshot.workers.push_back(worker_snapshot);
        }
    }

    auto& map = state.map_world.current_level().map;
    snapshot.map_tiles.reserve(map_tiles.size());
    for(const auto& [x, y] : map_tiles) {
        MapTileSnapshot tile;
        tile.x = x;
        tile.y = y;
        tile.is_wall = map.is_wall(x, y);
        snapshot.map_tiles.push_back(tile);
    }

    snapshot.has_code = state.software_manager.has_code();
    auto& current_code = state.software_manager.get();
    snapshot.current_label_count = current_code.labels.size();
    snapshot.current_code.assign(current_code.code.begin(),
                                 current_code.code.end());

    if(snapshot.worker_count > 0) {
        auto& ant_code = state.software_manager[0];
        snapshot.ant_label_count = ant_code.labels.size();
        snapshot.ant_code.assign(ant_code.code.begin(), ant_code.code.end());
    }

    return snapshot;
}

void assert_world_state(EngineState& state,
                        const WorldSnapshot& expected) {
    const auto& player = state.entity_manager.player.get_data();
    EXPECT_EQ(player.x, expected.player_x);
    EXPECT_EQ(player.y, expected.player_y);
    EXPECT_EQ(state.entity_manager.player_depth, expected.player_depth);
    EXPECT_EQ(state.map_world.current_depth, expected.current_depth);

    std::vector<Worker*> workers;
    workers.reserve(expected.worker_count);
    for(auto& level : state.map_world.levels) {
        for(auto* worker : level.workers) {
            workers.push_back(worker);
        }
    }

    ASSERT_EQ(workers.size(), expected.workers.size());
    EXPECT_EQ(workers.size(), expected.worker_count);
    for(size_t i = 0; i < workers.size(); ++i) {
        auto* worker = workers[i];
        const auto& cpu = worker->cpu;
        const auto& snapshot = expected.workers[i];
        EXPECT_EQ(worker->get_data().x, snapshot.x);
        EXPECT_EQ(worker->get_data().y, snapshot.y);
        EXPECT_EQ(cpu.registers[0], snapshot.reg_a);
        EXPECT_EQ(cpu.registers[1], snapshot.reg_b);
        EXPECT_EQ(cpu.zero_flag, snapshot.zero_flag);
        EXPECT_EQ(cpu.instr_failed_flag, snapshot.instr_failed_flag);
        EXPECT_EQ(cpu.dir_flag1, snapshot.dir_flag1);
        EXPECT_EQ(cpu.dir_flag2, snapshot.dir_flag2);
        EXPECT_EQ(cpu.is_move_flag, snapshot.is_move_flag);
        EXPECT_EQ(cpu.is_dig_flag, snapshot.is_dig_flag);
        EXPECT_EQ(cpu.instr_ptr_register, snapshot.instr_ptr);
        EXPECT_EQ(worker->program_executor.instr_trigger,
                  snapshot.instr_trigger);
    }

    auto& map = state.map_world.current_level().map;
    for(const auto& tile : expected.map_tiles) {
        EXPECT_EQ(map.is_wall(tile.x, tile.y), tile.is_wall);
    }

    EXPECT_EQ(state.software_manager.has_code(), expected.has_code);
    const auto& current_code = state.software_manager.get();
    std::vector<unsigned char> actual_current(current_code.code.begin(),
                                              current_code.code.end());
    EXPECT_EQ(actual_current, expected.current_code);
    EXPECT_EQ(current_code.labels.size(), expected.current_label_count);

    if(expected.worker_count > 0) {
        const auto& ant_code = state.software_manager[0];
        std::vector<unsigned char> actual_ant(ant_code.code.begin(),
                                              ant_code.code.end());
        EXPECT_EQ(actual_ant, expected.ant_code);
        EXPECT_EQ(ant_code.labels.size(), expected.ant_label_count);
    }
}

}  // namespace e2e
