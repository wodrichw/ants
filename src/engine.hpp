#ifndef __ENGINE_HPP
#define __ENGINE_HPP

#include <SDL_events.h>
#include <SDL_keycode.h>
#include <libtcod.hpp>
#include <libtcod/console.hpp>
#include <vector>

#include "controller.hpp"
#include "text_editor_handler.hpp"
#include "render.hpp"

namespace ant {
    class Ant;
    class Player;
}
class Building;
class ClockController;
class Map;
class ButtonController;

class Engine {
public :
    enum GameStatus {
        STARTUP,
        IDLE,
        TEXT_EDITOR,
        NEW_TURN,
        VICTORY,
        DEFEAT
    } gameStatus;

    struct InputEvent {
        long dx, dy; // keyboard move events
        std::optional<ulong> clickX, clickY; // mouse click events
    };

    std::vector<Ant*> ants;
    Map* map;
    Player* player;
    std::vector<Building*> buildings;
    std::vector<ClockController*> clockControllers;
    tcodRenderer renderer;
    TextEditorHandler editor;
    ButtonController* buttonController;
    ParserCommandsAssembler assembler;

    ulong clock_timeout_1000ms;

    Engine();
    ~Engine();
    void update();
    void render();
private:
    void handleMouseClick(SDL_MouseButtonEvent event);
    void handleKeyPress(SDL_Keycode key_sym, long& dx, long& dy);
};

extern Engine engine;

#endif //__ENGINE_HPP
