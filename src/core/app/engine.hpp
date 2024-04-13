#ifndef __ENGINE_HPP
#define __ENGINE_HPP

#include <SDL_events.h>
#include <SDL_keycode.h>
#include <stddef.h>

#include <libtcod.hpp>
#include <libtcod/console.hpp>
#include <vector>

#include "app/arg_parse.hpp"
#include "hardware/controller.hpp"
#include "ui/render.hpp"
#include "ui/text_editor_handler.hpp"

namespace ant {
    class Ant;
    class Player;
}  // namespace ant
class Building;
class ClockController;
class Map;
class ButtonController;

class Engine {
   public:
    enum GameStatus {
        STARTUP,
        IDLE,
        TEXT_EDITOR,
        NEW_TURN,
        VICTORY,
        DEFEAT
    } gameStatus;

    struct InputEvent {
        long dx, dy;                          // keyboard move events
        std::optional<ulong> clickX, clickY;  // mouse click events
    };

    ProjectArguments& config;
    std::vector<Ant*> ants;
    BoxManager box_manager;
    Map* map;
    Player* player;
    std::vector<Building*> buildings;
    std::vector<ClockController*> clockControllers;
    Renderer* renderer;
    TextEditorHandler editor;
    ButtonController* buttonController;
    ParserCommandsAssembler assembler;

    ulong clock_timeout_1000ms;

    Engine(ProjectArguments& config);
    ~Engine();
    void update();
    void render();

   private:
    void handleMouseClick(SDL_MouseButtonEvent event);
    void handleKeyPress(SDL_Keycode key_sym, long& dx, long& dy);
};

extern Engine engine;

#endif  //__ENGINE_HPP
