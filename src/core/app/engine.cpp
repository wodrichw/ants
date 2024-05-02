#include "engine.hpp"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <SDL_keycode.h>
#include <libtcod/context.h>
#include <libtcod/context_init.h>

#include <libtcod.hpp>
#include <libtcod/color.hpp>
#include <libtcod/console.hpp>
#include <libtcod/context.hpp>

#include "hardware/controller.hpp"
#include "app/globals.hpp"
#include "ui/buttonController.hpp"
#include "ui/ui_handlers.hpp"
#include "spdlog/spdlog.h"

Engine::Engine(ProjectArguments& config)
    : config(config),
        box_manager(globals::COLS, globals::ROWS),
        clockControllers(),
        renderer(config.is_render ? static_cast<Renderer*>(new tcodRenderer()) : static_cast<Renderer*>(new NoneRenderer())),
        editor(),
        entity_manager(box_manager.map_box->get_width(), box_manager.map_box->get_height(), config),
        eventSystem(),
        clock_timeout_1000ms(SDL_GetTicks64()) {

    SPDLOG_DEBUG("Setting game status to STARTUP");
    gameStatus = STARTUP;

    add_listeners();

    SPDLOG_DEBUG("Engine initialized");
}

Engine::~Engine() {
    SPDLOG_INFO("Destructing engine");
    delete buttonController;
    SPDLOG_TRACE("Engine destructed");
}

void Engine::add_listeners() {

    // player movement listeners
    // TODO: the player cant move when the editor is active - pass in a callback to the player to decide if the editor
    eventSystem.keyboard_events.add(LEFT_KEY_EVENT, new MoveLeftHandler(entity_manager.map, entity_manager.player, editor.is_active));
    eventSystem.keyboard_events.add(RIGHT_KEY_EVENT, new MoveRightHandler(entity_manager.map, entity_manager.player, editor.is_active));
    eventSystem.keyboard_events.add(UP_KEY_EVENT, new MoveUpHandler(entity_manager.map, entity_manager.player, editor.is_active));
    eventSystem.keyboard_events.add(DOWN_KEY_EVENT, new MoveDownHandler(entity_manager.map, entity_manager.player, editor.is_active));

    eventSystem.keyboard_events.add(H_KEY_EVENT, new MoveLeftHandler(entity_manager.map, entity_manager.player, editor.is_active));
    eventSystem.keyboard_events.add(L_KEY_EVENT, new MoveRightHandler(entity_manager.map, entity_manager.player, editor.is_active));
    eventSystem.keyboard_events.add(K_KEY_EVENT, new MoveUpHandler(entity_manager.map, entity_manager.player, editor.is_active));
    eventSystem.keyboard_events.add(J_KEY_EVENT, new MoveDownHandler(entity_manager.map, entity_manager.player, editor.is_active));
    eventSystem.keyboard_events.add(A_KEY_EVENT, new CreateAntHandler(entity_manager, clockControllers, editor));

    // click listeners
    eventSystem.mouse_events.add(LEFT_MOUSE_EVENT, new ClickHandler(entity_manager.map, *renderer, editor.is_active));

    // text editor listeners
    eventSystem.keyboard_events.add(RETURN_KEY_EVENT, new NewLineHandler(editor));
    eventSystem.keyboard_events.add(BACKSPACE_KEY_EVENT, new BackspaceHandler(editor));
    eventSystem.keyboard_events.add(LEFT_KEY_EVENT, new MoveCursorLeftHandler(editor));
    eventSystem.keyboard_events.add(RIGHT_KEY_EVENT, new MoveCursorRightHandler(editor));
    eventSystem.keyboard_events.add(UP_KEY_EVENT, new MoveCursorUpHandler(editor));
    eventSystem.keyboard_events.add(DOWN_KEY_EVENT, new MoveCursorDownHandler(editor));
    eventSystem.keyboard_events.add(SLASH_KEY_EVENT, new TextEditorTriggerHandler(editor));

    // text editor printable char listener
    eventSystem.char_keyboard_events.add(CHAR_KEY_EVENT, new EditorKeyHandler(editor));

}

void Engine::update() {
    // SPDLOG_TRACE("Updating engine");

    eventSystem.update();

    // SPDLOG_TRACE("Checking for clock pulse");
    if(clock_timeout_1000ms < SDL_GetTicks64()) {
        // SPDLOG_TRACE("Detected clock pulse");
        for(ClockController* c : clockControllers) {
            c->handleClockPulse();
        }
        clock_timeout_1000ms += 1000;
    }

    // SPDLOG_TRACE("Engine update complete");
}

void Engine::render() {
    // SPDLOG_TRACE("Rendering engine");
    LayoutBox& map_box = *box_manager.map_box;

    entity_manager.compute_fov();

    // draw the map
    renderer->renderMap(map_box, entity_manager.map);


    // draw the buildings
    // SPDLOG_TRACE("Rendering {} buildings", buildings.size());
    for(auto building : entity_manager.buildings) {
        renderer->renderBuilding(map_box, *building);
    }

    // draw the ants
    // SPDLOG_TRACE("Rendering {} ants", ants.size());
    for(auto ant : entity_manager.ants) {
        renderer->renderAnt(map_box, entity_manager.map, ant->get_data());
    }

    if( editor.is_active ){
        // SPDLOG_TRACE("Rendering text editor");
        renderer->renderTextEditor(*box_manager.text_editor_content_box, editor,
                                  entity_manager.ants.size());
    }

    // renderer->renderHelpBoxes();

    renderer->present();
    // SPDLOG_TRACE("Render complete");
}

