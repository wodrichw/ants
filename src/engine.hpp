#ifndef __ENGINE_HPP
#define __ENGINE_HPP

#include <SDL_events.h>
#include <SDL_keycode.h>
#include <libtcod.hpp>
#include <libtcod/console.hpp>
#include <vector>
#include "controller.hpp"
#include "map.hpp"
#include "ant.hpp"
#include "building.hpp"



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
        int dx, dy; // keyboard move events
        std::optional<ulong> clickX, clickY; // mouse click events
    };

    tcod::Context context;
    ant::Player* player;
    std::vector<ant::Ant*> ants;
    std::vector<Building*> buildings;
    std::vector<Controller*> controllers;
    Map *map;
    int fovRadius = 10;
    bool computeFov = true;

    ulong clock_timeout_1000ms;

    Engine();
    ~Engine();
    void update();
    void render();
private:
    static const int textBoxHeight = 17;
    static const int textBoxWidth = 23;
    static const int regBoxWidth = 6;
    static const int regBoxHeight = 1;
    std::vector<std::string> textEditorLines;
    int cursorX = 0, cursorY = 0;
    void printTextEditor();
    void printHelpBoxes();
    void handleTextEditorAction(SDL_Keycode key_sym);
    void handleMouseClick(SDL_MouseButtonEvent event);
    void handleKeyPress(SDL_Keycode key_sym, int& dx, int& dy);
    void moveToPrevNonWhiteSpace();
    void moveToEndLine();
    void moveAnt(ant::Ant* ant, int dx, int dy);
};

extern Engine engine;

#endif //__ENGINE_HPP
