#include "ui/replay.hpp"

#include <fstream>

#include <google/protobuf/message.h>

#include "utils/serializer.hpp"

namespace {
constexpr int kSizeBytes = sizeof(int);

ReplayError make_error(std::string message) {
    ReplayError err;
    err.message = std::move(message);
    return err;
}

bool read_size(std::ifstream& input, int& size) {
    input.read(reinterpret_cast<char*>(&size), kSizeBytes);
    return static_cast<bool>(input);
}

ReplayStatus read_message(std::ifstream& input,
                          google::protobuf::Message& msg,
                          const std::string& context) {
    int size = 0;
    if(!read_size(input, size)) {
        if(input.eof()) {
            ReplayError err = make_error("Reached end of replay stream");
            return ReplayStatus::failure(err);
        }
        ReplayError err =
            make_error("Failed to read replay size header for " + context);
        return ReplayStatus::failure(err);
    }

    if(size < 0) {
        ReplayError err = make_error("Invalid replay message size: " +
                                     std::to_string(size));
        return ReplayStatus::failure(err);
    }

    if(size == 0) {
        if(input.peek() == std::ifstream::traits_type::eof()) {
            ReplayError err = make_error("Invalid replay message size: " +
                                         std::to_string(size));
            return ReplayStatus::failure(err);
        }
        std::string data;
        if(!msg.ParseFromString(data)) {
            ReplayError err =
                make_error("Failed to parse replay message for " + context);
            return ReplayStatus::failure(err);
        }
        return ReplayStatus::success();
    }

    std::string data(size, '\0');
    input.read(data.data(), size);
    if(!input) {
        ReplayError err =
            make_error("Failed to read replay message body for " + context);
        return ReplayStatus::failure(err);
    }

    if(!msg.ParseFromString(data)) {
        ReplayError err =
            make_error("Failed to parse replay message for " + context);
        return ReplayStatus::failure(err);
    }

    return ReplayStatus::success();
}

std::string event_kind_to_string(ant_proto::ReplayEventKind kind) {
    switch(kind) {
        case ant_proto::REPLAY_EVENT_MOUSE_BUTTON_DOWN:
            return "mouse_button_down";
        case ant_proto::REPLAY_EVENT_KEY_DOWN:
            return "key_down";
        case ant_proto::REPLAY_EVENT_KEY_UP:
            return "key_up";
        case ant_proto::REPLAY_EVENT_CHAR:
            return "char";
        case ant_proto::REPLAY_EVENT_QUIT:
            return "quit";
        default:
            return "unknown";
    }
}
}  // namespace

ReplayStatus ReplayRecorder::start(const std::string& path,
                                  const ant_proto::ReplayHeader& header) {
    stop();
    packer = std::make_unique<Packer>(path);
    if(!packer || !(*packer)) {
        ReplayError err = make_error("Failed to open replay file for writing: " +
                                     path);
        last_status = ReplayStatus::failure(err);
        packer.reset();
        return last_status;
    }

    *packer << header;
    last_status = ReplayStatus::success();
    return last_status;
}

void ReplayRecorder::begin_frame(uint64_t frame_index) {
    if(!is_recording()) return;
    current_frame.Clear();
    current_frame.set_frame_index(frame_index);
    frame_open = true;
}

void ReplayRecorder::record_mouse_button_down(int64_t x, int64_t y,
                                              int32_t button) {
    if(!frame_open) return;
    ant_proto::ReplayEvent event;
    event.set_kind(ant_proto::REPLAY_EVENT_MOUSE_BUTTON_DOWN);
    auto* mouse = event.mutable_mouse();
    mouse->set_x(x);
    mouse->set_y(y);
    mouse->set_button(button);
    push_event(event);
}

void ReplayRecorder::record_key_down(int32_t key_sym, int32_t key_mod) {
    if(!frame_open) return;
    ant_proto::ReplayEvent event;
    event.set_kind(ant_proto::REPLAY_EVENT_KEY_DOWN);
    auto* key = event.mutable_key();
    key->set_key_sym(key_sym);
    key->set_key_mod(key_mod);
    push_event(event);
}

void ReplayRecorder::record_key_up(int32_t key_sym, int32_t key_mod) {
    if(!frame_open) return;
    ant_proto::ReplayEvent event;
    event.set_kind(ant_proto::REPLAY_EVENT_KEY_UP);
    auto* key = event.mutable_key();
    key->set_key_sym(key_sym);
    key->set_key_mod(key_mod);
    push_event(event);
}

void ReplayRecorder::record_char(uint32_t key) {
    if(!frame_open) return;
    ant_proto::ReplayEvent event;
    event.set_kind(ant_proto::REPLAY_EVENT_CHAR);
    auto* ch = event.mutable_ch();
    ch->set_key(key);
    push_event(event);
}

void ReplayRecorder::record_quit() {
    if(!frame_open) return;
    ant_proto::ReplayEvent event;
    event.set_kind(ant_proto::REPLAY_EVENT_QUIT);
    push_event(event);
}

void ReplayRecorder::end_frame() {
    if(!is_recording() || !frame_open) return;
    *packer << current_frame;
    frame_open = false;
}

void ReplayRecorder::stop() {
    packer.reset();
    frame_open = false;
}

bool ReplayRecorder::is_recording() const { return packer != nullptr; }

void ReplayRecorder::push_event(const ant_proto::ReplayEvent& event) {
    *current_frame.add_events() = event;
}

ReplayStatus ReplayPlayer::load(const std::string& path) {
    frames.clear();
    next_index = 0;
    loaded = false;

    std::ifstream input(path, std::ios::binary);
    if(!input) {
        ReplayError err = make_error("Failed to open replay file for reading: " +
                                     path);
        return ReplayStatus::failure(err);
    }

    auto header_status = read_message(input, header_msg, "replay header");
    if(!header_status.ok) {
        return header_status;
    }

    if(header_msg.version() != kReplayVersion) {
        ReplayError err = make_error(
            "Replay version mismatch. Expected " +
            std::to_string(kReplayVersion) + ", got " +
            std::to_string(header_msg.version()));
        err.expected_version = kReplayVersion;
        err.actual_version = header_msg.version();
        return ReplayStatus::failure(err);
    }

    while(true) {
        if(input.peek() == std::ifstream::traits_type::eof()) break;
        ant_proto::ReplayFrame frame;
        auto frame_status = read_message(input, frame, "replay frame");
        if(!frame_status.ok) {
            return frame_status;
        }
        frames.push_back(frame);
    }

    loaded = true;
    return ReplayStatus::success();
}

ReplayStatus load_replay_header(const std::string& path,
                                ant_proto::ReplayHeader& header) {
    std::ifstream input(path, std::ios::binary);
    if(!input) {
        ReplayError err = make_error("Failed to open replay file for reading: " +
                                     path);
        return ReplayStatus::failure(err);
    }

    auto header_status = read_message(input, header, "replay header");
    if(!header_status.ok) {
        return header_status;
    }

    if(header.version() != kReplayVersion) {
        ReplayError err = make_error(
            "Replay version mismatch. Expected " +
            std::to_string(kReplayVersion) + ", got " +
            std::to_string(header.version()));
        err.expected_version = kReplayVersion;
        err.actual_version = header.version();
        return ReplayStatus::failure(err);
    }

    return ReplayStatus::success();
}

bool ReplayPlayer::next_frame(uint64_t expected_frame_index,
                              ant_proto::ReplayFrame& frame,
                              ReplayError& error) {
    if(is_done()) {
        return false;
    }

    const auto& next_frame = frames[next_index];
    if(next_frame.frame_index() != expected_frame_index) {
        error.message = "Replay frame mismatch. Expected frame " +
                        std::to_string(expected_frame_index) +
                        ", got " +
                        std::to_string(next_frame.frame_index());
        error.frame_index = expected_frame_index;
        return false;
    }

    frame = next_frame;
    next_index++;

    if(!frame.events().empty()) {
        error.event_kind =
            event_kind_to_string(frame.events(0).kind());
    }

    return true;
}
