#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <SDL_keycode.h>
#include <libtcod/context.h>
#include <libtcod/context_init.h>
#include <libtcod.hpp>
#include <libtcod/color.hpp>
#include <libtcod/console.hpp>
#include <libtcod/context.hpp>
#include <google/protobuf/util/json_util.h>

#include "engine.pb.h"
#include "app/engine_state.hpp"
#include "app/engine.hpp"
#include "app/globals.hpp"
#include "ui/serializer_handler.hpp"
#include "spdlog/spdlog.h"
#include "ui/text_editor_handler.hpp"
#include "utils/thread_pool.hpp"

EngineState::EngineState(ProjectArguments& config, Renderer* renderer):
    box_manager(globals::COLS, globals::ROWS),
    job_pool(8),
    map_world(Rect(0, 0, box_manager.map_box->get_width(), box_manager.map_box->get_height()), config.is_walls_enabled),
    map_manager(globals::COLS*2,  globals::ROWS*2, config, map_world),
    entity_manager(map_manager, map_world, map_world.current_level().start_info->player_x, map_world.current_level().start_info->player_y, job_pool),
    software_manager(command_map),
    primary_mode(*box_manager.map_box, command_map, software_manager, entity_manager, map_manager, map_world, *renderer, is_reload_game, job_pool),
    editor_mode(*renderer, *box_manager.text_editor_content_box, software_manager, map_world.levels),
    state(&primary_mode, &editor_mode)
{
    
    SPDLOG_INFO("Creating engine state");
    add_listeners(config);
    SPDLOG_INFO("Engine initialized without backup");
}

EngineState::EngineState(const ant_proto::EngineState& msg, ProjectArguments& config, Renderer* renderer):
    box_manager(globals::COLS, globals::ROWS),
    job_pool(8),
    map_world(msg.map_world(), job_pool, config.is_walls_enabled),
    map_manager(msg.map_manager(), map_world),
    entity_manager(msg.entity_manager(), map_manager, map_world, job_pool),
    software_manager(msg.software_manger(), command_map),
    primary_mode(msg.hardware_manager(), *box_manager.map_box, command_map, software_manager, entity_manager, map_manager, map_world, *renderer, is_reload_game, job_pool),
    editor_mode(*renderer, *box_manager.text_editor_content_box, software_manager, map_world.levels),
    state(&primary_mode, &editor_mode) {
    
    add_listeners(config);
    SPDLOG_INFO("Engine initialized with backup");
}

EngineState::~EngineState() {
    SPDLOG_INFO("Destructing engine state");
    SPDLOG_TRACE("Engine state destructed");
}

void EngineState::add_listeners(ProjectArguments& config) {
    SPDLOG_DEBUG("Adding root event system subscriptions");
    root_event_system.keyboard_events.add(SLASH_KEY_EVENT,
                                          new TextEditorTriggerHandler(state));
    root_event_system.keyboard_events.add(BACK_SLASH_KEY_EVENT,
                                          new AutoSaveTriggerHandler(*this, config.save_path));
}

void EngineState::update() {
    // SPDLOG_TRACE("Updating engine");

    SDL_Event event;
    MouseEvent mouse_event;
    KeyboardEvent keyboard_event;
    CharKeyboardEvent char_keyboard_event;

    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                handle_quit_event();
                break;

            case SDL_MOUSEBUTTONDOWN:
                set_mouse_type(event.button, mouse_event);

                root_event_system.mouse_events.notify(mouse_event);
                state.get_mouse_publisher().notify(mouse_event);
                break;
            
            case SDL_KEYUP:
                unset_keyboard_chord_type(event.key.keysym, keyboard_chord_event);
                break;

            case SDL_KEYDOWN:
                set_keyboard_chord_type(event.key.keysym, keyboard_chord_event);
                root_event_system.keyboard_chord_events.notify(keyboard_chord_event);
                state.get_keyboard_chord_publisher().notify(keyboard_chord_event);

                set_keyboard_type(event.key.keysym, keyboard_event);
                root_event_system.keyboard_events.notify(keyboard_event);
                state.get_keyboard_publisher().notify(keyboard_event);

                set_char_keyboard_type(event.key.keysym, char_keyboard_event);
                root_event_system.char_keyboard_events.notify(char_keyboard_event);
                state.get_char_keyboard_publisher().notify(char_keyboard_event);
                break;
        }
    }
    state.update();

    // SPDLOG_TRACE("Engine state update complete");
}

void EngineState::render() {
    state.render();
}

Packer& operator<<(Packer& p, EngineState const& obj) {
    ant_proto::EngineState msg;
    *msg.mutable_entity_manager() = obj.entity_manager.get_proto();
    *msg.mutable_software_manger() = obj.software_manager.get_proto();
    *msg.mutable_hardware_manager() = obj.primary_mode.get_proto();
    *msg.mutable_map_world() = obj.map_world.get_proto();
    *msg.mutable_map_manager() = obj.map_manager.get_proto();
    

    // UNCOMMMENT BELOW FOR DEBUGGING SERIALIZATION
    //
    // std::string json_msg;
    // google::protobuf::util::MessageToJsonString(msg, &json_msg);
    // std::ofstream json_f("write.json");
    // json_f << json_msg;

    return p << msg;
}

