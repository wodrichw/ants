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
    : player(new ant::Player(40, 25, 10, '@', color::white)), ants({player}),
    buildings(), clockControllers(), map(new Map(globals::COLS, globals::ROWS, ants, buildings)),
    renderer(), editor(map), buttonController(new ButtonController()), clock_timeout_1000ms(SDL_GetTicks64())
{
    gameStatus = STARTUP;
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
    size_t x = 0, y = 0;
    renderer.pixel_to_tile_coordinates(event.x, event.y, x, y);
    buttonController->handleClick(x, y);
}

void Engine::handleKeyPress(SDL_Keycode key_sym, int& dx, int& dy)
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

        Building &b = *buildings[player->bldgId.value()];
        size_t addAnt_x = b.x, addAnt_y = b.y;
        ant::Worker* new_ant = new ant::Worker(addAnt_x, addAnt_y);
        ButtonController::Button* new_button = new ButtonController::Button{
            addAnt_x, addAnt_y, 1, 1, // button lives on top of the ant we are adding
            ButtonController::Layer::FIFTH, // button lives on bottom layer and thus last priority to be clicked
            [new_ant]() {
                if( new_ant->col == color::light_green ) new_ant->col = color::dark_yellow;
                else new_ant->col = color::light_green;
                return true;
            },
            std::optional<tcod::ColorRGB>()
        };
        EngineInteractor interactor;
        interactor.move_ant = [&, new_ant, new_button](int dx, int dy) {
            if( map->canWalk(new_ant->x + dx, new_ant->y + dy) &&
                buttonController->canMoveButton(new_button, dx, dy))
            {
                moveAnt(new_ant, dx, dy);
                buttonController->moveButton(new_button, dx, dy);
            }
        };

        Worker_Controller* w = new Worker_Controller(assembler, interactor,  editor.textEditorLines);

        if (! interactor.status.p_err ) { // add worker ant if the textEditorLines have no parser errors
            clockControllers.push_back(w);
            ants.push_back(new_ant);
            buttonController->addButton(new_button);
        } else {
            delete w;
            delete new_ant;
            // TODO: show parse errors in the text editor box instead of a cout
            // this will likely require returning the line number, and word that caused the parse error
            std::cout << interactor.status.err_msg << std::endl;
        }

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

void Engine::moveAnt(ant::Ant *ant, int dx, int dy) {
    ant->updatePositionByDelta(dx, dy);
    map->updateFov();

    if (map->getTile(ant->x, ant->y).bldgId.has_value()) {
        ant->bldgId.emplace(map->getTile(ant->x, ant->y).bldgId.value());
    } else {
        ant->bldgId.reset();
    }
}

void Engine::update() {
    if (gameStatus == STARTUP) {
        map->updateFov();
        render();
    }
    if (gameStatus != TEXT_EDITOR)
        gameStatus = IDLE;

    SDL_Event event;
    int dx = 0, dy = 0;
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
        moveAnt(player, dx, dy);
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
        renderer.renderTextEditor(editor);

    renderer.renderHelpBoxes();

    renderer.present();
}
