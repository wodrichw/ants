#pragma once

#include "hardware/program_executor.hpp"
#include "ui/text_editor.hpp"
#include "ui/event_system.hpp"
#include "hardware/software_manager.hpp"
#include "entity/entity_manager.hpp"
#include "app/game_state.hpp"
#include "app/game_mode.hpp"
#include "app/arg_parse.hpp"

class Renderer;
class ProjectArugments;
class Unpacker;
struct KeyboardEvent;

struct EngineState {
    BoxManager box_manager;
    ThreadPool<AsyncProgramJob> job_pool;
    EntityManager entity_manager;
    EventSystem root_event_system = {};
    CommandMap command_map = {};
    SoftwareManager software_manager;

    PrimaryMode primary_mode;
    EditorMode editor_mode;
    GameState state;

    KeyboardChordEvent keyboard_chord_event = {};
    bool is_reload_game = false;

   public:

    EngineState(ProjectArguments&, Renderer*);
    EngineState(Unpacker&, ProjectArguments&, Renderer*);
    ~EngineState();
    void update();
    void render();

   private:
    void add_listeners(ProjectArguments&);
    friend Packer& operator<<(Packer&, EngineState const&);
};
