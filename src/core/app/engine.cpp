#include "app/engine.hpp"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <SDL_keycode.h>
#include <libtcod/context.h>
#include <libtcod/context_init.h>

#include <libtcod.hpp>
#include <libtcod/color.hpp>
#include <libtcod/console.hpp>
#include <libtcod/context.hpp>

#include "app/globals.hpp"
#include "spdlog/spdlog.h"

Engine::Engine(ProjectArguments& config)
    : box_manager(globals::COLS, globals::ROWS),
      renderer(config.is_render ? static_cast<Renderer*>(new tcodRenderer(
                                      config.is_debug_graphics))
                                : static_cast<Renderer*>(new NoneRenderer())),
      entity_manager(box_manager.map_box->get_width(),
                     box_manager.map_box->get_height(), config),
      root_event_system(),
      software_manager(command_map),
      primary_mode(*box_manager.map_box, command_map, software_manager, entity_manager, *renderer),
      editor_mode(*renderer, *box_manager.text_editor_content_box, software_manager, entity_manager.ants),
      state(&primary_mode, &editor_mode) {

    SPDLOG_DEBUG("Setting game status to STARTUP");
    add_listeners();
    SPDLOG_DEBUG("Engine initialized");
}

Engine::~Engine() {
    SPDLOG_INFO("Destructing engine");
    SPDLOG_TRACE("Engine destructed");
}

void Engine::add_listeners() {
    root_event_system.keyboard_events.add(SLASH_KEY_EVENT,
                                          new TextEditorTriggerHandler(state));
}

void Engine::update() {
    // SPDLOG_TRACE("Updating engine");

    SDL_Event event;
    MouseEvent mouse_event;
    KeyboardEvent keyboad_event;
    CharKeyboardEvent char_keyboad_event;

    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                handle_quit_event();
                break;

            case SDL_MOUSEBUTTONDOWN:
                get_mouse_event(event.button, mouse_event);
                root_event_system.mouse_events.notify(mouse_event);
                state.get_mouse_publisher().notify(mouse_event);
                break;

            case SDL_KEYDOWN:
                get_keyboard_event(event.key.keysym, keyboad_event);
                root_event_system.keyboard_events.notify(keyboad_event);
                state.get_keyboard_publisher().notify(keyboad_event);

                get_char_keyboard_event(event.key.keysym, char_keyboad_event);
                root_event_system.char_keyboard_events.notify(
                    char_keyboad_event);
                state.get_char_keyboard_publisher().notify(char_keyboad_event);
                break;
        }
    }
    state.update();

    // SPDLOG_TRACE("Engine update complete");
}

void Engine::render() {
    state.render();
    renderer->present();

    // SPDLOG_TRACE("Render complete");
}
