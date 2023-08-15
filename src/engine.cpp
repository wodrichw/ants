#include <cstdlib>
#include <libtcod.hpp>
#include <libtcod/color.hpp>
#include <libtcod/console.hpp>
#include <libtcod/context.h>
#include <libtcod/context.hpp>
#include <libtcod/context_init.h>
#include <SDL2/SDL_events.h>
#include "engine.hpp"
#include "ant.hpp"

static const int ROWS = 60;
static const int COLS = 60;

Engine::Engine(): 
    player(new ant::Player(40,25, 10, '@', tcod::ColorRGB{255,255,255})),
    ants( {player} ),
    map(new Map(COLS, ROWS, ants))
{
    gameStatus = STARTUP;
    auto params = TCOD_ContextParams();
    params.columns = COLS, params.rows = ROWS, params.window_title = "A N T S";
    context = tcod::Context(params);
    map->root_console = context.new_console(COLS, ROWS);
}

Engine::~Engine() {
    for (auto ant : ants) delete ant;
    delete map;
}

void Engine::update() {
    if ( gameStatus == STARTUP ) { 
        map->updateFov();
        render();
    }
    gameStatus=IDLE;

    SDL_Event event;
    int dx = 0, dy = 0;
    while ( SDL_PollEvent(&event) ) {
        switch (event.type) {
            case SDL_QUIT:
                std::exit(EXIT_SUCCESS);
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_h || event.key.keysym.sym == SDLK_LEFT) {
                    dx = -1;
                } else if (event.key.keysym.sym == SDLK_j || event.key.keysym.sym == SDLK_DOWN) {
                    dy = 1;
                } else if (event.key.keysym.sym == SDLK_k || event.key.keysym.sym == SDLK_UP) {
                    dy = -1;
                } else if (event.key.keysym.sym == SDLK_l || event.key.keysym.sym == SDLK_RIGHT) {
                    dx = 1;
                }
                break;
        }
    }

    if ( dx != 0 || dy != 0 ) {
        gameStatus=NEW_TURN;
        if ( map->canWalk(player->x + dx, player->y + dy) ) {
            map->clearCh(player->x, player->y);
            player->updatePositionByDelta(dx, dy);
            map->updateFov();
        }
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
    // draw the map
    map->render();

    // draw the ants
    for (auto ant: ants) {
        map->renderAnt(*ant);
    }
    context.present(map->root_console);
}


