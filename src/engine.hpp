#ifndef __ENGINE_HPP
#define __ENGINE_HPP

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
};

extern Engine engine;

#endif //__ENGINE_HPP
