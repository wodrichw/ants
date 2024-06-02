#ifndef __ENGINE_HPP
#define __ENGINE_HPP

#include "app/arg_parse.hpp"
#include "app/game_state.hpp"
#include "app/game_mode.hpp"
#include "ui/text_editor.hpp"
#include "hardware/parser.hpp"
#include "entity/entity_manager.hpp"
#include "ui/event_system.hpp"
#include "hardware/software_manager.hpp"

using ulong = unsigned long;

class Renderer;

class Engine {
    BoxManager box_manager; 
    Renderer* renderer;
    EntityManager entity_manager;
    EventSystem root_event_system;
    CommandMap command_map;
    SoftwareManager software_manager;

    PrimaryMode primary_mode;
    EditorMode editor_mode;
    GameState state;

   public:

    Engine(ProjectArguments& config);
    Engine(Unpacker&, ProjectArguments&);
    ~Engine();
    void update();
    void render();
    friend Packer& operator<<(Packer&, Engine const&);

   private:
    void add_listeners();
};

extern Engine engine;

#endif  //__ENGINE_HPP
