#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <SDL_keycode.h>
#include <cstdlib>
#include <libtcod.hpp>
#include <libtcod/color.hpp>
#include <libtcod/console.hpp>
#include <libtcod/context.h>
#include <libtcod/context.hpp>
#include <libtcod/context_init.h>
#include <iostream>

#include "ant.hpp"
#include "building.hpp"
#include "buttonController.hpp"
#include "colors.hpp"
#include "controller.hpp"
#include "engine.hpp"
#include "globals.hpp"
#include "map.hpp"

Engine::Engine()
    : ants(), map(new Map(globals::COLS, globals::ROWS, ants, buildings)),
    player(new Player(map, 40, 25, 10, '@', color::white)),
    buildings(), clockControllers(),  renderer(), editor(map), buttonController(new ButtonController()),
    clock_timeout_1000ms(SDL_GetTicks64())
{
    gameStatus = STARTUP;
    ants.push_back(player);
    map->build();
}

Engine::~Engine()
{
    for (auto ant : ants)
        delete ant;
    delete map;
    delete buttonController;
}

struct Column {
    int width, height;
};

// Find location of mouse click and iterate through z-index from top to bottom
// to see if it lands on anything selectable
void Engine::handleMouseClick(SDL_MouseButtonEvent event)
{
    if ( event.button != SDL_BUTTON_LEFT ) return;
    long x = 0, y = 0;
    renderer.pixel_to_tile_coordinates(event.x, event.y, x, y);
    buttonController->handleClick(x, y);
}

void Engine::handleKeyPress(SDL_Keycode key_sym, long& dx, long& dy)
{
    if (key_sym == SDLK_SLASH && gameStatus == TEXT_EDITOR) {
        gameStatus = IDLE;
        return;
    }

    if (key_sym == SDLK_SLASH && gameStatus != TEXT_EDITOR) {
        gameStatus = TEXT_EDITOR;
        return;
    }

    if (gameStatus == TEXT_EDITOR) {
        editor.handleTextEditorAction(key_sym);
        return;
    }

    if (key_sym == SDLK_a && player->bldgId.has_value()) {
        // Make worker
        // TODO: make an intelligent location picker for workers (addAnt_x,addAnt_y)
        //   -- should look around building and scan for an open location
        //   -- if no location present increase the scan radius (as if the building
        //   absorbs its perimiter squares)
        //      and look for open squared there. Radius increasing will go on until
        //      an open square is found (or out of space in the map)

        Worker_Controller* w = new Worker_Controller(assembler,  editor.textEditorLines);
        if (w->parser.status.p_err) {
            // TODO: show parse errors in the text editor box instead of a cout
            // this will likely require returning the line number, and word that caused the parse error
            std::cout << w->parser.status.err_msg << std::endl;
            return;
        }

        Building &b = *buildings[player->bldgId.value()];
        long addAnt_x = b.x + b.w / 2, addAnt_y = b.y + b.h / 2;
        Worker* new_ant = new Worker(map, buttonController, addAnt_x, addAnt_y);

        w->ant_interactor.try_move = [new_ant](long dx, long dy) {
            if (new_ant->can_move(dx, dy)) new_ant->move(dx, dy);
        };
        w->ant_interactor.read_register = [new_ant](long idx)-> cpu_word_size const& {
            return new_ant->cpu.registers[idx];
        };
        w->ant_interactor.write_register = [new_ant](long idx, cpu_word_size value) {
            new_ant->cpu.registers[idx] = value;
        };

        ants.push_back(new_ant);
        clockControllers.push_back(w);

        return;
    }

    // update player location input (only if no other keys were pressed)
    if (key_sym == SDLK_h || key_sym == SDLK_LEFT) {
        dx = -1;
    } else if (key_sym == SDLK_j || key_sym == SDLK_DOWN) {
        dy = 1;
    } else if (key_sym == SDLK_k || key_sym == SDLK_UP) {
        dy = -1;
    } else if (key_sym == SDLK_l || key_sym == SDLK_RIGHT) {
        dx = 1;
    }
}

void Engine::update()
{
    if (gameStatus == STARTUP) {
        map->updateFov();
        render();
    }
    if (gameStatus != TEXT_EDITOR)
        gameStatus = IDLE;

    SDL_Event event;
    long dx = 0, dy = 0;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                std::exit(EXIT_SUCCESS);

            case SDL_MOUSEBUTTONDOWN:
                handleMouseClick(event.button);
                break;

            case SDL_KEYDOWN:
                // fix keypress syms to account for shift key
                if( event.key.keysym.mod & (KMOD_RSHIFT | KMOD_LSHIFT) ) {
                    if( event.key.keysym.sym == SDLK_3 ) event.key.keysym.sym = SDLK_HASH;
                    if( event.key.keysym.sym == SDLK_SEMICOLON ) event.key.keysym.sym = SDLK_COLON;
                }
                handleKeyPress(event.key.keysym.sym, dx, dy);
                break;
        }
    }


    if (clock_timeout_1000ms < SDL_GetTicks64()) {
        for (ClockController *c : clockControllers) {
            c->handleClockPulse();
        }
        clock_timeout_1000ms += 1000;
    }

    if ((dx != 0 || dy != 0) &&
        map->canWalk(player->x + dx, player->y + dy))
    {
        player->move(dx, dy);
    }
}

void Engine::render()
{
    // draw the map
    renderer.renderMap(*map);

    // draw the ants
    for (auto ant : ants) {
        renderer.renderAnt(*map, *ant);
    }

    // draw the buildings
    for (auto building : buildings) {
        renderer.renderBuilding(*building);
    }

    if (gameStatus == TEXT_EDITOR)
        renderer.renderTextEditor(editor, ants.size());

    renderer.renderHelpBoxes();

    renderer.present();
}
