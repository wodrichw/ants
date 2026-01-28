#pragma once

#include <functional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "app/arg_parse.hpp"
#include "app/engine.hpp"
#include "app/engine_state.hpp"
#include "replay.pb.h"
#include "ui/replay.hpp"
#include "ui/render.hpp"
#include "utils/serializer.hpp"

#ifndef E2E_ASSET_DIR
#define E2E_ASSET_DIR "tests/test_assets"
#endif

namespace e2e {

struct E2eCase {
    std::string name;
    std::function<void()> run;
};

std::ostream& operator<<(std::ostream& os, const E2eCase& spec);

std::string replay_dir();
std::string make_replay_path(const std::string& name);

ant_proto::ReplayHeader make_header(uint32_t version = kReplayVersion,
                                    uint32_t seed_x = 100,
                                    uint32_t seed_y = 200,
                                    bool walls_enabled = false,
                                    const std::string& map_path = "");

ant_proto::ReplayEvent make_mouse_event(int64_t x, int64_t y, int32_t button);

ant_proto::ReplayEvent make_key_event(ant_proto::ReplayEventKind kind,
                                      int32_t key_sym,
                                      int32_t key_mod);

ant_proto::ReplayEvent make_char_event(uint32_t key);

ant_proto::ReplayEvent make_bare_event(ant_proto::ReplayEventKind kind);

ant_proto::ReplayFrame make_frame(
    uint64_t frame_index,
    const std::vector<ant_proto::ReplayEvent>& events);

void write_replay(const std::string& path,
                  const ant_proto::ReplayHeader& header,
                  const std::vector<ant_proto::ReplayFrame>& frames);

void write_replay_with_bad_frame_size(const std::string& path,
                                      const ant_proto::ReplayHeader& header,
                                      int size);

void write_replay_with_truncated_frame(const std::string& path,
                                       const ant_proto::ReplayHeader& header,
                                       int size,
                                       int bytes_written);

struct ReplayRunResult {
    bool has_error = false;
    bool done = false;
    ReplayError error = {};
};

ReplayRunResult run_replay(const std::string& path, size_t max_frames = 120);

struct ActionWorld {
    ProjectArguments config;
    NoneRenderer renderer;
    EngineState state;

    explicit ActionWorld(bool walls_enabled);
};

std::vector<std::pair<long, long>> collect_offsets(Map& map,
                                                   long x,
                                                   long y,
                                                   bool want_wall,
                                                   size_t count);

}  // namespace e2e
