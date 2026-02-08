#pragma once

#include "app/arg_parse.hpp"
#include "app/clock_speed.hpp"
#include "app/game_mode.hpp"
#include "app/game_state.hpp"
#include "engine.pb.h"
#include "entity/entity_manager.hpp"
#include "hardware/program_executor.hpp"
#include "hardware/software_manager.hpp"
#include "map/manager.hpp"
#include "ui/event_system.hpp"
#include "ui/replay.hpp"
#include "ui/sidebar_menu.hpp"
#include "ui/text_editor.hpp"

#include <optional>
#include <string>
#include <vector>

class Renderer;
class ProjectArugments;
struct KeyboardEvent;

struct EngineState {
    Renderer& renderer;
    BoxManager box_manager;
    ThreadPool<AsyncProgramJob> job_pool;
    MapWorld map_world;
    MapManager map_manager;
    EntityManager entity_manager;
    EventSystem root_event_system = {};
    CommandMap command_map = {};
    SoftwareManager software_manager;

    PrimaryMode primary_mode;
    EditorMode editor_mode;
    GameState state;

    SidebarMenu sidebar_menu;

    KeyboardChordEvent keyboard_chord_event = {};
    ClockSpeed clock_speed = ClockSpeed::NORMAL;
    bool is_reload_game = false;

    ReplayRecorder replay_recorder = {};
    ReplayPlayer replay_player = {};
    bool is_replay_recording = false;
    bool is_replay_playing = false;
    bool is_replay_complete = false;
    uint64_t replay_frame_index = 0;
    std::optional<ReplayError> replay_error = std::nullopt;

    std::string save_path;

   public:
    EngineState(ProjectArguments&, Renderer*);
    EngineState(ProjectArguments&, Renderer*,
                const ant_proto::ReplayEnvironment* env);
    EngineState(const ant_proto::EngineState&, ProjectArguments&, Renderer*);
    ~EngineState();
    void update();
    void render();
    void toggle_sidebar();
    void handle_sidebar_action(SidebarMenuAction action);

    void configure_replay(ProjectArguments& config);
    ReplayStatus start_replay_recording(const std::string& path,
                                        ProjectArguments& config);
    ReplayStatus start_replay_playback(const std::string& path);
    void stop_replay_recording();

    bool replay_has_error() const { return replay_error.has_value(); }
    ReplayError replay_last_error() const {
        return replay_error.value_or(ReplayError{});
    }
    bool replay_done() const { return is_replay_complete; }

    void action_move_player(long dx, long dy);
    void action_dig(long dx, long dy);
    void action_create_ant();
    void action_add_program_lines(const std::vector<std::string>& lines);
    void action_assign_program_to_ant(ulong ant_idx);
    void action_go_up();
    void action_go_down();

   private:
    void add_listeners(ProjectArguments&);
    void dispatch_mouse_event(const MouseEvent& mouse_event);
    void dispatch_key_down(SDL_Keysym const& key_sym,
                           KeyboardEvent& keyboard_event,
                           CharKeyboardEvent& char_keyboard_event);
    void dispatch_key_up(SDL_Keysym const& key_sym);
    void dispatch_char_event(uint32_t key);
    friend class KeyboardE2eTestAccess;
    friend Packer& operator<<(Packer&, EngineState const&);
};
