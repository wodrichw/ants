#include "app/facade.hpp"

#include <SDL_timer.h>

AntGameFacade::AntGameFacade() : engine(), clock_timeout(SDL_GetTicks64()) {}
AntGameFacade::AntGameFacade(int argc, char* argv[])
    : engine(argc, argv), clock_timeout(SDL_GetTicks64()) {}
AntGameFacade::AntGameFacade(ProjectArguments& config)
    : engine(config), clock_timeout(SDL_GetTicks64()) {}

bool AntGameFacade::update() {
    if(clock_timeout >= SDL_GetTicks64()) return false;
    ulong interval_ms =  17 * 2;  // 1000ms / 60 FPS = 17
    if(engine.get_clock_speed() == ClockSpeed::FAST) {
        interval_ms = 17 / 2;
    }
    clock_timeout += interval_ms;

    engine.update();
    engine.render();
    return true;
}

void AntGameFacade::engine_update() { engine.update(); }

ReplayStatus AntGameFacade::start_replay_recording(const std::string& path) {
    return engine.start_replay_recording(path);
}

ReplayStatus AntGameFacade::start_replay_playback(const std::string& path) {
    return engine.start_replay_playback(path);
}

void AntGameFacade::stop_replay_recording() { engine.stop_replay_recording(); }

bool AntGameFacade::replay_has_error() const { return engine.replay_has_error(); }

ReplayError AntGameFacade::replay_last_error() const {
    return engine.replay_last_error();
}

bool AntGameFacade::replay_done() const { return engine.replay_done(); }

void AntGameFacade::action_move_player(long dx, long dy) {
    engine.action_move_player(dx, dy);
}

void AntGameFacade::action_dig(long dx, long dy) {
    engine.action_dig(dx, dy);
}

void AntGameFacade::action_create_ant() { engine.action_create_ant(); }

void AntGameFacade::action_add_program_lines(
    const std::vector<std::string>& lines) {
    engine.action_add_program_lines(lines);
}

void AntGameFacade::action_assign_program_to_ant(ulong ant_idx) {
    engine.action_assign_program_to_ant(ant_idx);
}

void AntGameFacade::action_go_up() { engine.action_go_up(); }

void AntGameFacade::action_go_down() { engine.action_go_down(); }
