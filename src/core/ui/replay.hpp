#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "replay.pb.h"
#include "utils/serializer.hpp"

struct ReplayError {
    std::string message = {};
    uint32_t expected_version = 0;
    uint32_t actual_version = 0;
    uint64_t frame_index = 0;
    uint32_t event_index = 0;
    std::string event_kind = {};
};

struct ReplayStatus {
    bool ok = true;
    ReplayError error = {};

    static ReplayStatus success() { return ReplayStatus{}; }
    static ReplayStatus failure(ReplayError err) {
        ReplayStatus status;
        status.ok = false;
        status.error = std::move(err);
        return status;
    }
};

inline constexpr uint32_t kReplayVersion = 1;

ReplayStatus load_replay_header(const std::string& path,
                                ant_proto::ReplayHeader& header);

class ReplayRecorder {
   public:
    ReplayStatus start(const std::string& path,
                       const ant_proto::ReplayHeader& header);
    void begin_frame(uint64_t frame_index);
    void record_mouse_button_down(int64_t x, int64_t y, int32_t button);
    void record_key_down(int32_t key_sym, int32_t key_mod);
    void record_key_up(int32_t key_sym, int32_t key_mod);
    void record_char(uint32_t key);
    void record_quit();
    void end_frame();
    void stop();

    bool is_recording() const;
    const ReplayStatus& status() const { return last_status; }

   private:
    std::unique_ptr<class Packer> packer = nullptr;
    ant_proto::ReplayFrame current_frame = {};
    bool frame_open = false;
    ReplayStatus last_status = ReplayStatus::success();

    void push_event(const ant_proto::ReplayEvent& event);
};

class ReplayPlayer {
   public:
    ReplayStatus load(const std::string& path);
    bool is_loaded() const { return loaded; }
    bool is_done() const { return loaded && next_index >= frames.size(); }

    bool next_frame(uint64_t expected_frame_index, ant_proto::ReplayFrame& frame,
                    ReplayError& error);

    const ant_proto::ReplayHeader& header() const { return header_msg; }

   private:
    ant_proto::ReplayHeader header_msg = {};
    std::vector<ant_proto::ReplayFrame> frames = {};
    size_t next_index = 0;
    bool loaded = false;
};
