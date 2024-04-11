#include "engine.hpp"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <SDL_keycode.h>
#include <libtcod/context.h>
#include <libtcod/context_init.h>

#include <cstdlib>
#include <libtcod.hpp>
#include <libtcod/color.hpp>
#include <libtcod/console.hpp>
#include <libtcod/context.hpp>

#include "entity/ant.hpp"
#include "entity/building.hpp"
#include "ui/buttonController.hpp"
#include "ui/colors.hpp"
#include "hardware/controller.hpp"
#include "app/globals.hpp"
#include "entity/map.hpp"
#include "spdlog/spdlog.h"

Engine::Engine(ProjectArguments& config)
    : config(config),
      ants(),
      box_manager(globals::COLS, globals::ROWS),
      map(new Map(box_manager.map_box->get_width(),
                  box_manager.map_box->get_height(), ants, buildings, config)),
      player(new Player(map, 40, 25, 10, '@', color::white)),
      buildings(),
      clockControllers(),
      renderer(config.is_render ? static_cast<Renderer*>(new tcodRenderer()) : static_cast<Renderer*>(new NoneRenderer())),
      editor(map),
      buttonController(new ButtonController()),
      clock_timeout_1000ms(SDL_GetTicks64()) {

    SPDLOG_DEBUG("Setting game status to STARTUP");
    gameStatus = STARTUP;

    SPDLOG_TRACE("Adding player to the list of ants");
    ants.push_back(player);

    map->build();
    SPDLOG_DEBUG("Engine initialized");
}

Engine::~Engine() {
    SPDLOG_INFO("Destructing engine");

    SPDLOG_DEBUG("Deleting {} ants", ants.size());
    for(auto ant : ants) delete ant;
    delete map;
    delete buttonController;
    SPDLOG_TRACE("Engine destructed");
}

struct Column {
    int width, height;
};

// Find location of mouse click and iterate through z-index from top to bottom
// to see if it lands on anything selectable
void Engine::handleMouseClick(SDL_MouseButtonEvent event) {
    SPDLOG_DEBUG("Detected mouse click");
    if(event.button != SDL_BUTTON_LEFT) return;

    SPDLOG_INFO("Left mouse click at ({}, {})", event.x, event.y);
    long x = 0, y = 0;
    renderer->pixel_to_tile_coordinates(event.x, event.y, x, y);
    buttonController->handleClick(x, y);
    SPDLOG_TRACE("Mouse click handled");
}

void Engine::handleKeyPress(SDL_Keycode key_sym, long& dx, long& dy) {
    if(key_sym == SDLK_SLASH && gameStatus == TEXT_EDITOR) {
        SPDLOG_INFO("Detected '/' key press, exiting text editor");
        gameStatus = IDLE;
        return;
    }

    if(key_sym == SDLK_SLASH && gameStatus != TEXT_EDITOR) {
        SPDLOG_INFO("Detected '/' key press, starting up text editor");
        gameStatus = TEXT_EDITOR;
        return;
    }

    if(gameStatus == TEXT_EDITOR) {
        editor.handleTextEditorAction(key_sym);
        return;
    }

    if(key_sym == SDLK_a && player->bldgId.has_value()) {
        // Make worker
        // TODO: make an intelligent location picker for workers
        // (addAnt_x,addAnt_y)
        //   -- should look around building and scan for an open location
        //   -- if no location present increase the scan radius (as if the
        //   building absorbs its perimiter squares)
        //      and look for open squared there. Radius increasing will go on
        //      until an open square is found (or out of space in the map)

        SPDLOG_INFO("Detected 'a' key press, adding worker ant");
        Building& b = *buildings[player->bldgId.value()];
        long new_x = b.x + b.w / 2, new_y = b.y + b.h / 2;
        SPDLOG_DEBUG("Selected building at ({}, {})", new_x, new_y);

        ButtonController::ButtonData btn_data{new_x, new_y, 1, 1,
                                              ButtonController::Layer::FIFTH};
        if(!buttonController->canCreateButton(btn_data)){
            SPDLOG_WARN("Cannot create button at ({}, {})", new_x, new_y);
            return;
        }

        SPDLOG_DEBUG("Creating worker controller");
        Worker_Controller* w =
            new Worker_Controller(assembler, editor.textEditorLines);
        if(w->parser.status.p_err) {
            // TODO: show parse errors in the text editor box instead of a cout
            // this will likely require returning the line number, and word that
            // caused the parse error
            SPDLOG_ERROR("Error parsing worker controller: {}",
                         w->parser.status.err_msg);
            return;
        }

        SPDLOG_DEBUG("Creating worker ant");
        Worker* new_ant = new Worker(map, buttonController, btn_data);

        SPDLOG_TRACE("Setting ant interactor functions");
        w->ant_interactor.try_move = [new_ant](long dx, long dy) {
            if(new_ant->can_move(dx, dy)) new_ant->move(dx, dy);
        };
        w->ant_interactor.read_register =
            [new_ant](long idx) -> cpu_word_size const& {
            SPDLOG_TRACE("Reading register {} -> {}", idx, new_ant->cpu.registers[idx]);
            return new_ant->cpu.registers[idx];
        };
        w->ant_interactor.write_register = [new_ant](long idx,
                                                     cpu_word_size value) {
            SPDLOG_TRACE("Writing register {} <- {}", idx, value);
            new_ant->cpu.registers[idx] = value;
        };
        w->ant_interactor.read_zero_flag = [new_ant]() -> bool const& {
            SPDLOG_TRACE("Reading zero flag -> {}", new_ant->cpu.zero_flag);
            return new_ant->cpu.zero_flag;
        };
        w->ant_interactor.write_zero_flag = [new_ant](bool flag) {
            SPDLOG_TRACE("Writing zero flag <- {}", flag);
            new_ant->cpu.zero_flag = flag;
        };

        ants.push_back(new_ant);
        clockControllers.push_back(w);

        return;
    }

    // update player location input (only if no other keys were pressed)
    if(key_sym == SDLK_h || key_sym == SDLK_LEFT) {
        SPDLOG_DEBUG("Detected 'left' key press");
        dx = -1;
    } else if(key_sym == SDLK_j || key_sym == SDLK_DOWN) {
        SPDLOG_DEBUG("Detected 'down' key press");
        dy = 1;
    } else if(key_sym == SDLK_k || key_sym == SDLK_UP) {
        SPDLOG_DEBUG("Detected 'up' key press");
        dy = -1;
    } else if(key_sym == SDLK_l || key_sym == SDLK_RIGHT) {
        SPDLOG_DEBUG("Detected 'right' key press");
        dx = 1;
    }
    SPDLOG_TRACE("Key press handled");
}

void Engine::update() {
    // SPDLOG_TRACE("Updating engine");
    if(gameStatus == STARTUP) {
        SPDLOG_TRACE("Detected game status STARTUP");
        map->updateFov();
        render();
    }
    if(gameStatus != TEXT_EDITOR && gameStatus != IDLE) {
        SPDLOG_DEBUG("Editor is not on - setting game status to IDLE");
        gameStatus = IDLE;
    }

    SDL_Event event;
    long dx = 0, dy = 0;
    // SPDLOG_TRACE("Polling for events");
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                SPDLOG_INFO("Detected SDL_QUIT event, exiting");
                std::exit(EXIT_SUCCESS);

            case SDL_MOUSEBUTTONDOWN:
                handleMouseClick(event.button);
                break;

            case SDL_KEYDOWN:
                // fix keypress syms to account for shift key
                SPDLOG_TRACE("Handling key press event");
                if(event.key.keysym.mod & (KMOD_RSHIFT | KMOD_LSHIFT)) {
                    if(event.key.keysym.sym == SDLK_3)
                        event.key.keysym.sym = SDLK_HASH;
                    if(event.key.keysym.sym == SDLK_SEMICOLON)
                        event.key.keysym.sym = SDLK_COLON;
                }
                handleKeyPress(event.key.keysym.sym, dx, dy);
                break;
        }
    }

    // SPDLOG_TRACE("Checking for clock pulse");
    if(clock_timeout_1000ms < SDL_GetTicks64()) {
        // SPDLOG_TRACE("Detected clock pulse");
        for(ClockController* c : clockControllers) {
            c->handleClockPulse();
        }
        clock_timeout_1000ms += 1000;
    }

    // SPDLOG_TRACE("Checking for player movement");
    if((dx != 0 || dy != 0) && map->canWalk(player->x + dx, player->y + dy)) {
        SPDLOG_DEBUG("Player can move to ({}, {})", player->x + dx,
                     player->y + dy);
        player->move(dx, dy);
    }
    // SPDLOG_TRACE("Engine update complete");
}

void Engine::render() {
    // SPDLOG_TRACE("Rendering engine");
    LayoutBox& map_box = *box_manager.map_box;

    // draw the map
    renderer->renderMap(map_box, *map);

    // draw the ants
    // SPDLOG_TRACE("Rendering {} ants", ants.size());
    for(auto ant : ants) {
        renderer->renderAnt(map_box, *map, *ant);
    }

    // draw the buildings
    // SPDLOG_TRACE("Rendering {} buildings", buildings.size());
    for(auto building : buildings) {
        renderer->renderBuilding(map_box, *building);
    }

    if(gameStatus == TEXT_EDITOR){
        // SPDLOG_TRACE("Rendering text editor");
        renderer->renderTextEditor(*box_manager.text_editor_content_box, editor,
                                  ants.size());
    }

    // renderer->renderHelpBoxes();

    renderer->present();
    // SPDLOG_TRACE("Render complete");
}
