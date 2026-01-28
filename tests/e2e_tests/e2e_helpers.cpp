#include "e2e_helpers.hpp"

#include <fstream>
#include <ostream>

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

}  // namespace e2e
