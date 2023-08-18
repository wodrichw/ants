#ifndef __ENGINE_HPP
#define __ENGINE_HPP

#include <SDL_keycode.h>
#include <libtcod.hpp>
#include <libtcod/console.hpp>
#include <vector>
#include "map.hpp"
#include "ant.hpp"



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

    tcod::Context context;
    ant::Player* player;
    std::vector<ant::Ant*> ants;
    Map *map;
    int fovRadius = 10;
    bool computeFov = true;

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
    void handleKeyPress(SDL_Keycode key_sym, int& dx, int& dy);
    void moveToPrevNonWhiteSpace();
    void moveToEndLine();
    
};

extern Engine engine;

#endif //__ENGINE_HPP
