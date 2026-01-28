#include <gtest/gtest.h>

#include <functional>
#include <string>
#include <vector>

#include "e2e_helpers.hpp"

namespace {

using e2e::E2eCase;

std::vector<E2eCase> build_replay_cases() {
    std::vector<E2eCase> cases;

    auto add_case = [&](const std::string& name,
                        std::function<void()> run) {
        cases.push_back(E2eCase{name, std::move(run)});
    };

    // Area 1: Replay version mismatch (5 cases)
    {
        const std::vector<uint32_t> versions = {0, 2, 7, 99, 1234};
        for(size_t i = 0; i < versions.size(); ++i) {
            const std::string name =
                "replay_version_mismatch_" + std::to_string(i + 1);
            const uint32_t version = versions[i];
            add_case(name, [name, version]() {
                const auto path = e2e::make_replay_path(name);
                e2e::write_replay(path, e2e::make_header(version), {});
                const auto result = e2e::run_replay(path);
                EXPECT_TRUE(result.has_error);
                EXPECT_NE(result.error.message.find("Replay version mismatch"),
                          std::string::npos);
            });
        }
    }

    // Area 2: Replay frame index mismatch (5 cases)
    {
        const std::vector<std::vector<uint64_t>> sequences = {
            {1}, {2}, {5}, {0, 2}, {0, 1, 3}};
        for(size_t i = 0; i < sequences.size(); ++i) {
            const std::string name =
                "replay_frame_index_mismatch_" + std::to_string(i + 1);
            const auto seq = sequences[i];
            add_case(name, [name, seq]() {
                const auto path = e2e::make_replay_path(name);
                std::vector<ant_proto::ReplayFrame> frames;
                for(const auto index : seq) {
                    frames.push_back(e2e::make_frame(index, {}));
                }
                e2e::write_replay(path, e2e::make_header(), frames);
                const auto result = e2e::run_replay(path, 10);
                EXPECT_TRUE(result.has_error);
                EXPECT_NE(result.error.message.find("Replay frame mismatch"),
                          std::string::npos);
            });
        }
    }

    // Area 3: Missing key down payload (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name =
            "replay_missing_key_down_payload_" + std::to_string(i + 1);
        add_case(name, [name]() {
            const auto path = e2e::make_replay_path(name);
            auto frame = e2e::make_frame(
                0, {e2e::make_bare_event(ant_proto::REPLAY_EVENT_KEY_DOWN)});
            e2e::write_replay(path, e2e::make_header(), {frame});
            const auto result = e2e::run_replay(path, 3);
            EXPECT_TRUE(result.has_error);
            EXPECT_NE(result.error.message.find("missing key payload"),
                      std::string::npos);
        });
    }

    // Area 4: Missing key up payload (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name =
            "replay_missing_key_up_payload_" + std::to_string(i + 1);
        add_case(name, [name]() {
            const auto path = e2e::make_replay_path(name);
            auto frame = e2e::make_frame(
                0, {e2e::make_bare_event(ant_proto::REPLAY_EVENT_KEY_UP)});
            e2e::write_replay(path, e2e::make_header(), {frame});
            const auto result = e2e::run_replay(path, 3);
            EXPECT_TRUE(result.has_error);
            EXPECT_NE(result.error.message.find("missing key payload"),
                      std::string::npos);
        });
    }

    // Area 5: Missing mouse payload (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name =
            "replay_missing_mouse_payload_" + std::to_string(i + 1);
        add_case(name, [name]() {
            const auto path = e2e::make_replay_path(name);
            auto frame = e2e::make_frame(
                0, {e2e::make_bare_event(
                        ant_proto::REPLAY_EVENT_MOUSE_BUTTON_DOWN)});
            e2e::write_replay(path, e2e::make_header(), {frame});
            const auto result = e2e::run_replay(path, 3);
            EXPECT_TRUE(result.has_error);
            EXPECT_NE(result.error.message.find("missing mouse payload"),
                      std::string::npos);
        });
    }

    // Area 6: Missing char payload (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name =
            "replay_missing_char_payload_" + std::to_string(i + 1);
        add_case(name, [name]() {
            const auto path = e2e::make_replay_path(name);
            auto frame = e2e::make_frame(
                0, {e2e::make_bare_event(ant_proto::REPLAY_EVENT_CHAR)});
            e2e::write_replay(path, e2e::make_header(), {frame});
            const auto result = e2e::run_replay(path, 3);
            EXPECT_TRUE(result.has_error);
            EXPECT_NE(result.error.message.find("missing char payload"),
                      std::string::npos);
        });
    }

    // Area 7: Invalid replay message size (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name =
            "replay_invalid_message_size_" + std::to_string(i + 1);
        const int size = (i % 2 == 0) ? 0 : -1;
        add_case(name, [name, size]() {
            const auto path = e2e::make_replay_path(name);
            e2e::write_replay_with_bad_frame_size(path, e2e::make_header(), size);
            const auto result = e2e::run_replay(path, 3);
            EXPECT_TRUE(result.has_error);
            EXPECT_NE(result.error.message.find("Invalid replay message size"),
                      std::string::npos);
        });
    }

    // Area 8: Truncated replay frame body (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name =
            "replay_truncated_frame_" + std::to_string(i + 1);
        const int size = 16 + i;
        const int bytes_written = 4 + i;
        add_case(name, [name, size, bytes_written]() {
            const auto path = e2e::make_replay_path(name);
            e2e::write_replay_with_truncated_frame(path, e2e::make_header(), size,
                                                   bytes_written);
            const auto result = e2e::run_replay(path, 3);
            EXPECT_TRUE(result.has_error);
            EXPECT_NE(
                result.error.message.find("Failed to read replay message body"),
                std::string::npos);
        });
    }

    // Area 9: Empty replay (no frames) completes (5 cases)
    for(int i = 0; i < 5; ++i) {
        const std::string name = "replay_empty_" + std::to_string(i + 1);
        add_case(name, [name]() {
            const auto path = e2e::make_replay_path(name);
            e2e::write_replay(path, e2e::make_header(), {});
            const auto result = e2e::run_replay(path, 1);
            EXPECT_FALSE(result.has_error);
            EXPECT_TRUE(result.done);
        });
    }

    // Area 10: Large mixed event frames (5 cases)
    {
        const std::vector<size_t> counts = {1, 10, 25, 50, 100};
        for(size_t i = 0; i < counts.size(); ++i) {
            const std::string name =
                "replay_large_frame_" + std::to_string(i + 1);
            const size_t count = counts[i];
            add_case(name, [name, count]() {
                std::vector<ant_proto::ReplayEvent> events;
                events.reserve(count);
                for(size_t idx = 0; idx < count; ++idx) {
                    switch(idx % 4) {
                        case 0:
                            events.push_back(e2e::make_mouse_event(
                                static_cast<int64_t>(idx),
                                static_cast<int64_t>(idx + 1), 1));
                            break;
                        case 1:
                            events.push_back(e2e::make_key_event(
                                ant_proto::REPLAY_EVENT_KEY_DOWN,
                                static_cast<int32_t>('a' + (idx % 26)), 0));
                            break;
                        case 2:
                            events.push_back(e2e::make_key_event(
                                ant_proto::REPLAY_EVENT_KEY_UP,
                                static_cast<int32_t>('a' + (idx % 26)), 0));
                            break;
                        default:
                            events.push_back(e2e::make_char_event(
                                static_cast<uint32_t>('a' + (idx % 26))));
                            break;
                    }
                }

                const auto path = e2e::make_replay_path(name);
                auto frame = e2e::make_frame(0, events);
                e2e::write_replay(path, e2e::make_header(), {frame});

                const auto result = e2e::run_replay(path, 5);
                EXPECT_FALSE(result.has_error);
                EXPECT_TRUE(result.done);
            });
        }
    }

    return cases;
}

}  // namespace

class ReplayE2eTest : public ::testing::TestWithParam<E2eCase> {};

TEST_P(ReplayE2eTest, Runs) { GetParam().run(); }

INSTANTIATE_TEST_SUITE_P(E2eReplay,
                         ReplayE2eTest,
                         ::testing::ValuesIn(build_replay_cases()));
