#include <libtcod.hpp>
#include <libtcod/color.hpp>
#include <libtcod/context.h>
#include <libtcod/context_init.h>
#include "engine.hpp"

Engine::Engine(): 
    player(new Actor(40,25,'@', "white", tcod::ColorRGB{255,255,255})),
    map(new Map(80,45))
{
    TCODConsole::initRoot(80,50,"roguelike C++",false);
    actors.push(player);
}

Engine::~Engine() {
    actors.clearAndDelete();
    delete map;
}

void Engine::update() {
    TCOD_key_t key;
    if ( gameStatus == STARTUP ) map->computeFov();
    gameStatus=IDLE;
    TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS,&key,NULL);

    int dx=0,dy=0;
    switch(key.vk) {
        case TCODK_UP : dy=-1; break;
        case TCODK_DOWN : dy=1; break;
        case TCODK_LEFT : dx=-1; break;
        case TCODK_RIGHT : dx=1; break;
        default:break;
    }
    if ( dx != 0 || dy != 0 ) {
        gameStatus=NEW_TURN;
        if ( player->moveOrAttack(player->x+dx,player->y+dy) ) {
            map->computeFov();
        }
    }
    if ( gameStatus == NEW_TURN ) {
        for (Actor **iterator=actors.begin(); iterator != actors.end();
                iterator++) {
            Actor *actor=*iterator;
            if ( actor != player ) {
                actor->update();
            }
        }
    }
}

void Engine::render() {
    TCODConsole::root->clear();
    // draw the map
    map->render();

    // draw the actors
    for (auto actor: actors) {
        if ( map->isInFov(actor->x,actor->y) ) {
            actor->render();
        }
    }
}


