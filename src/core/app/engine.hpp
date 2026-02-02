#pragma once

#include "app/arg_parse.hpp"
#include "ui/replay.hpp"

#include <string>
#include <vector>

class Renderer;
struct EngineState;

class Engine {
    ProjectArguments config;
    Renderer* renderer = {};
    EngineState* state = {};

   public:
    Engine();
    Engine(int argc, char* argv[]);
    Engine(ProjectArguments& config);
    ~Engine();
    void update();
    void render();

    ReplayStatus start_replay_recording(const std::string& path);
    ReplayStatus start_replay_playback(const std::string& path);
    void stop_replay_recording();
    bool replay_has_error() const;
    ReplayError replay_last_error() const;
    bool replay_done() const;

    void action_move_player(long dx, long dy);
    void action_dig(long dx, long dy);
    void action_create_ant();
    void action_add_program_lines(const std::vector<std::string>& lines);
    void action_assign_program_to_ant(ulong ant_idx);
    void action_go_up();
    void action_go_down();

    EngineState* get_state();
    void reload_state_for_tests();

   private:
    void initialize();
    Renderer* create_renderer() const;
    EngineState* create_state();
};
