#ifndef __ENGINE_HPP
#define __ENGINE_HPP

#include <libtcod.hpp>
#include "map.hpp"
#include "actor.hpp"



class Engine {
public :
    enum GameStatus {
        STARTUP,
        IDLE,
        NEW_TURN,
        VICTORY,
        DEFEAT
    } gameStatus;

    TCODList<Actor *> actors;
    Actor *player;
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
