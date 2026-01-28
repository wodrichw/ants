#pragma once

#include "app/engine.hpp"

#include <string>
#include <vector>

using ulong = unsigned long;

class AntGameFacade {
   public:
    AntGameFacade();
    AntGameFacade(int argc, char* argv[]);
    AntGameFacade(ProjectArguments& config);

    bool update();
    void engine_update();

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

   private:
    Engine engine = {};
    ulong clock_timeout = {};
};