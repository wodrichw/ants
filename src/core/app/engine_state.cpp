#include "app/engine_state.hpp"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <SDL_keycode.h>
#include <google/protobuf/util/json_util.h>
#include <libtcod/context.h>
#include <libtcod/context_init.h>

#include <chrono>

#include <libtcod.hpp>
#include <libtcod/color.hpp>
#include <libtcod/console.hpp>
#include <libtcod/context.hpp>

#include "app/engine.hpp"
#include "app/globals.hpp"
#include "entity/entity_actions.hpp"
#include "engine.pb.h"
#include "replay.pb.h"
#include "spdlog/spdlog.h"
#include "ui/serializer_handler.hpp"
#include "ui/text_editor_handler.hpp"
#include "utils/thread_pool.hpp"

EngineState::EngineState(ProjectArguments& config, Renderer* renderer)
    : box_manager(globals::COLS, globals::ROWS),
      job_pool(8),
      map_world(Rect(0, 0, box_manager.map_box->get_width(),
                     box_manager.map_box->get_height()),
                config.is_walls_enabled),
      map_manager(globals::COLS * 2, globals::ROWS * 2, config, map_world),
      entity_manager(map_manager, map_world,
                     map_world.current_level().start_info->player_x,
                     map_world.current_level().start_info->player_y, job_pool),
      software_manager(command_map),
      primary_mode(*box_manager.map_box, command_map, software_manager,
                   entity_manager, map_manager, map_world, *renderer,
                   is_reload_game, job_pool),
      editor_mode(*renderer, *box_manager.text_editor_content_box,
                  software_manager, map_world.levels),
      state(&primary_mode, &editor_mode) {
    SPDLOG_INFO("Creating engine state");
    add_listeners(config);
    SPDLOG_INFO("Engine initialized without backup");
    configure_replay(config);
}

EngineState::EngineState(ProjectArguments& config, Renderer* renderer,
                                                 const ant_proto::ReplayEnvironment* env)
        : box_manager(globals::COLS, globals::ROWS),
            job_pool(8),
            map_world(Rect(0, 0, box_manager.map_box->get_width(),
                                         box_manager.map_box->get_height()),
                                config.is_walls_enabled,
                                env ? env->region_seed_x() : 0,
                                env ? env->region_seed_y() : 0),
            map_manager(globals::COLS * 2, globals::ROWS * 2, config, map_world),
            entity_manager(map_manager, map_world,
                                         map_world.current_level().start_info->player_x,
                                         map_world.current_level().start_info->player_y, job_pool),
            software_manager(command_map),
            primary_mode(*box_manager.map_box, command_map, software_manager,
                                     entity_manager, map_manager, map_world, *renderer,
                                     is_reload_game, job_pool),
            editor_mode(*renderer, *box_manager.text_editor_content_box,
                                    software_manager, map_world.levels),
            state(&primary_mode, &editor_mode) {
        SPDLOG_INFO("Creating engine state (seeded)");
        add_listeners(config);
        SPDLOG_INFO("Engine initialized without backup");
        configure_replay(config);
}

EngineState::EngineState(const ant_proto::EngineState& msg,
                         ProjectArguments& config, Renderer* renderer)
    : box_manager(globals::COLS, globals::ROWS),
      job_pool(8),
      map_world(msg.map_world(), job_pool, config.is_walls_enabled),
      map_manager(msg.map_manager(), map_world),
      entity_manager(msg.entity_manager(), map_manager, map_world, job_pool),
      software_manager(msg.software_manger(), command_map),
      primary_mode(msg.hardware_manager(), *box_manager.map_box, command_map,
                   software_manager, entity_manager, map_manager, map_world,
                   *renderer, is_reload_game, job_pool),
      editor_mode(*renderer, *box_manager.text_editor_content_box,
                  software_manager, map_world.levels),
      state(&primary_mode, &editor_mode) {
    add_listeners(config);
    SPDLOG_INFO("Engine initialized with backup");
    configure_replay(config);
}

EngineState::~EngineState() {
    SPDLOG_INFO("Destructing engine state");
    SPDLOG_TRACE("Engine state destructed");
}

void EngineState::add_listeners(ProjectArguments& config) {
    SPDLOG_DEBUG("Adding root event system subscriptions");
    root_event_system.keyboard_events.add(SLASH_KEY_EVENT,
                                          new TextEditorTriggerHandler(state));
    root_event_system.keyboard_events.add(
        BACK_SLASH_KEY_EVENT,
        new AutoSaveTriggerHandler(*this, config.save_path));
}

void EngineState::configure_replay(ProjectArguments& config) {
    if(!config.replay_play_path.empty() &&
       !config.replay_record_path.empty()) {
        ReplayError err;
        err.message =
            "Replay config error: cannot enable playback and recording "
            "simultaneously";
        replay_error = err;
        return;
    }

    if(!config.replay_play_path.empty()) {
        auto status = start_replay_playback(config.replay_play_path);
        if(!status.ok) replay_error = status.error;
    }

    if(!config.replay_record_path.empty()) {
        auto status = start_replay_recording(config.replay_record_path, config);
        if(!status.ok) replay_error = status.error;
    }
}

ReplayStatus EngineState::start_replay_recording(const std::string& path,
                                                 ProjectArguments& config) {
    if(is_replay_playing) {
        ReplayError err;
        err.message =
            "Cannot start replay recording while playback is active";
        replay_error = err;
        return ReplayStatus::failure(err);
    }

    ant_proto::ReplayHeader header;
    header.set_version(kReplayVersion);
    header.set_created_unix_ms(
        static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count()));
    header.set_build(std::string(__DATE__) + " " + __TIME__);

    auto* env = header.mutable_environment();
    uint32_t seed_x = 0;
    uint32_t seed_y = 0;
    map_world.get_origin_region_seeds(seed_x, seed_y);
    env->set_region_seed_x(seed_x);
    env->set_region_seed_y(seed_y);
    env->set_is_walls_enabled(config.is_walls_enabled);
    env->set_map_path(config.default_map_file_path);

    auto status = replay_recorder.start(path, header);
    if(status.ok) {
        is_replay_recording = true;
        replay_frame_index = 0;
        is_replay_complete = false;
    }
    return status;
}

ReplayStatus EngineState::start_replay_playback(const std::string& path) {
    if(is_replay_recording) {
        ReplayError err;
        err.message =
            "Cannot start replay playback while recording is active";
        replay_error = err;
        return ReplayStatus::failure(err);
    }

    auto status = replay_player.load(path);
    if(status.ok) {
        is_replay_playing = true;
        replay_frame_index = 0;
        is_replay_complete = false;
    }
    return status;
}

void EngineState::stop_replay_recording() {
    replay_recorder.stop();
    is_replay_recording = false;
}

void EngineState::dispatch_mouse_event(const MouseEvent& mouse_event) {
    root_event_system.mouse_events.notify(mouse_event);
    state.get_mouse_publisher().notify(mouse_event);
}

void EngineState::dispatch_key_down(SDL_Keysym const& key_sym,
                                    KeyboardEvent& keyboard_event,
                                    CharKeyboardEvent& char_keyboard_event) {
    Mode* mode_at_dispatch = state.mode;
    set_keyboard_chord_type(key_sym, keyboard_chord_event);
    root_event_system.keyboard_chord_events.notify(keyboard_chord_event);
    mode_at_dispatch->get_keyboard_chord_publisher().notify(
        keyboard_chord_event);

   set_keyboard_type(key_sym, keyboard_event);
    root_event_system.keyboard_events.notify(keyboard_event);
    mode_at_dispatch->get_keyboard_publisher().notify(keyboard_event);

    set_char_keyboard_type(key_sym, char_keyboard_event);
    root_event_system.char_keyboard_events.notify(char_keyboard_event);
    mode_at_dispatch->get_char_keyboard_publisher().notify(
        char_keyboard_event);
}

void EngineState::dispatch_key_up(SDL_Keysym const& key_sym) {
    unset_keyboard_chord_type(key_sym, keyboard_chord_event);
}

void EngineState::dispatch_char_event(uint32_t key) {
    CharKeyboardEvent char_keyboard_event;
    char_keyboard_event.key = static_cast<char>(key);
    root_event_system.char_keyboard_events.notify(char_keyboard_event);
    state.get_char_keyboard_publisher().notify(char_keyboard_event);
}

void EngineState::update() {
    // SPDLOG_TRACE("Updating engine");

    SDL_Event event;
    MouseEvent mouse_event;
    KeyboardEvent keyboard_event;
    CharKeyboardEvent char_keyboard_event;

    if(is_replay_recording) {
        replay_recorder.begin_frame(replay_frame_index);
    }

    if(is_replay_playing) {
        if(replay_player.is_done()) {
            is_replay_playing = false;
            is_replay_complete = true;
        } else {
            ant_proto::ReplayFrame frame;
            ReplayError err;
            if(!replay_player.next_frame(replay_frame_index, frame, err)) {
                replay_error = err;
                is_replay_playing = false;
                return;
            }

            for(int i = 0; i < frame.events_size(); ++i) {
                const auto& ev = frame.events(i);
                switch(ev.kind()) {
                    case ant_proto::REPLAY_EVENT_MOUSE_BUTTON_DOWN: {
                        if(!ev.has_mouse()) {
                            ReplayError payload;
                            payload.message =
                                "Replay event missing mouse payload";
                            payload.frame_index = replay_frame_index;
                            payload.event_index = static_cast<uint32_t>(i);
                            payload.event_kind = "mouse_button_down";
                            replay_error = payload;
                            is_replay_playing = false;
                            return;
                        }
                        MouseEvent m;
                        m.x = static_cast<long>(ev.mouse().x());
                        m.y = static_cast<long>(ev.mouse().y());
                        m.type = get_mouse_type(static_cast<char>(
                            ev.mouse().button()));
                        dispatch_mouse_event(m);
                        break;
                    }
                    case ant_proto::REPLAY_EVENT_KEY_DOWN: {
                        if(!ev.has_key()) {
                            ReplayError payload;
                            payload.message = "Replay event missing key payload";
                            payload.frame_index = replay_frame_index;
                            payload.event_index = static_cast<uint32_t>(i);
                            payload.event_kind = "key_down";
                            replay_error = payload;
                            is_replay_playing = false;
                            return;
                        }
                        SDL_Keysym sym = {};
                        sym.sym = ev.key().key_sym();
                        sym.mod = static_cast<Uint16>(ev.key().key_mod());
                        dispatch_key_down(sym, keyboard_event,
                                          char_keyboard_event);
                        break;
                    }
                    case ant_proto::REPLAY_EVENT_KEY_UP: {
                        if(!ev.has_key()) {
                            ReplayError payload;
                            payload.message = "Replay event missing key payload";
                            payload.frame_index = replay_frame_index;
                            payload.event_index = static_cast<uint32_t>(i);
                            payload.event_kind = "key_up";
                            replay_error = payload;
                            is_replay_playing = false;
                            return;
                        }
                        SDL_Keysym sym = {};
                        sym.sym = ev.key().key_sym();
                        sym.mod = static_cast<Uint16>(ev.key().key_mod());
                        dispatch_key_up(sym);
                        break;
                    }
                    case ant_proto::REPLAY_EVENT_CHAR: {
                        if(!ev.has_ch()) {
                            ReplayError payload;
                            payload.message = "Replay event missing char payload";
                            payload.frame_index = replay_frame_index;
                            payload.event_index = static_cast<uint32_t>(i);
                            payload.event_kind = "char";
                            replay_error = payload;
                            is_replay_playing = false;
                            return;
                        }
                        dispatch_char_event(ev.ch().key());
                        break;
                    }
                    case ant_proto::REPLAY_EVENT_QUIT:
                        handle_quit_event();
                        break;
                    default:
                        break;
                }
            }
        }
    } else {
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    if(is_replay_recording) {
                        replay_recorder.record_quit();
                    }
                    handle_quit_event();
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    set_mouse_type(event.button, mouse_event);
                    if(is_replay_recording) {
                        replay_recorder.record_mouse_button_down(
                            mouse_event.x, mouse_event.y, event.button.button);
                    }
                    dispatch_mouse_event(mouse_event);
                    break;

                case SDL_KEYUP:
                    if(is_replay_recording) {
                        replay_recorder.record_key_up(event.key.keysym.sym,
                                                     event.key.keysym.mod);
                    }
                    dispatch_key_up(event.key.keysym);
                    break;

                case SDL_KEYDOWN:
                    if(is_replay_recording) {
                        replay_recorder.record_key_down(event.key.keysym.sym,
                                                       event.key.keysym.mod);
                    }
                    dispatch_key_down(event.key.keysym, keyboard_event,
                                      char_keyboard_event);
                    break;
            }
        }
    }
    state.update();

    if(is_replay_recording) {
        replay_recorder.end_frame();
    }
    replay_frame_index++;

    // SPDLOG_TRACE("Engine state update complete");
}

void EngineState::action_move_player(long dx, long dy) {
    map_manager.move_entity(entity_manager.player_depth, dy, dx,
                            entity_manager.player);
}

void EngineState::action_dig(long dx, long dy) {
    handle_dig(map_world.current_level().map, entity_manager.player,
               entity_manager.player.inventory, dx, dy);
}

void EngineState::action_create_ant() {
    entity_manager.create_ant(primary_mode.get_hardware_manager(),
                              software_manager);
}

void EngineState::action_add_program_lines(
    const std::vector<std::string>& lines) {
    software_manager.add_lines(lines);
}

void EngineState::action_assign_program_to_ant(ulong ant_idx) {
    software_manager.assign(ant_idx);
}

void EngineState::action_go_up() { map_manager.go_up(); }

void EngineState::action_go_down() { map_manager.go_down(); }

void EngineState::render() { state.render(); }

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
