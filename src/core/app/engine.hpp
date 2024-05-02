#ifndef __ENGINE_HPP
#define __ENGINE_HPP

#include "app/arg_parse.hpp"
#include "app/game_state.hpp"
#include "app/game_mode.hpp"
#include "ui/text_editor.hpp"
#include "hardware/parser.hpp"
#include "entity/entity_manager.hpp"
#include "ui/event_system.hpp"

using ulong = unsigned long;

class Renderer;

class Engine {
    BoxManager box_manager;
    TextEditor editor;

    Renderer* renderer;
    ParserCommandsAssembler assembler;
    EntityManager entity_manager;
    EventSystem root_event_system;

    PrimaryMode primary_mode;
    EditorMode editor_mode;
    GameState state;

   public:

    Engine(ProjectArguments& config);
    ~Engine();
    void update();
    void render();

   private:
    void add_listeners();
};

extern Engine engine;

#endif  //__ENGINE_HPP
