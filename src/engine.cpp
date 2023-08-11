#include <libtcod.hpp>
#include <libtcod/color.hpp>
#include <libtcod/context.h>
#include <libtcod/context_init.h>
#include "engine.hpp"
#include "ant.hpp"

Engine::Engine(): 
    player(new ant::Player(40,25, 10, '@', tcod::ColorRGB{255,255,255})),
    ants( {player} ),
    map(new Map(80,45, ants))
{
    TCODConsole::initRoot(80,50,"A N T S",false);
    map->updateFov();
}

Engine::~Engine() {
    for (auto ant : ants) delete ant;
    delete map;
}

void Engine::update() {
    TCOD_key_t key;
    if ( gameStatus == STARTUP ) map->updateFov();
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
        if ( map->canWalk(player->x + dx, player->y + dy) ) {
            player->updatePositionByDelta(dx, dy);
            map->updateFov();
        }
        // if ( player->moveOrAttack(player->x+dx,player->y+dy) ) {
            // map->updateFov();
        // }
    }
    if ( gameStatus == NEW_TURN ) {
        // for (Actor **iterator=actors.begin(); iterator != actors.end();
        //         iterator++) {
        //     Actor *actor=*iterator;
        //     if ( actor != player ) {
        //         actor->update();
        //     }
        // }
    }
}


void Engine::render() 
{
    TCODConsole::root->clear();
    // draw the map
    map->render();

    // draw the ants
    for (auto ant: ants) {
        if ( map->isInFov(ant->x,ant->y) ) {
            ant->render();
        }
    }
}


