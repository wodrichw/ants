#include "app/engine.hpp"

#include "app/arg_parse.hpp"
#include "app/engine_state.hpp"
#include "engine.pb.h"
#include "replay.pb.h"
#include "spdlog/spdlog.h"
#include "ui/render.hpp"

Engine::Engine()
    : config(0, nullptr), renderer(create_renderer()), state(create_state()) {
    initialize();
}
Engine::Engine(int argc, char* argv[])
    : config(argc, argv), renderer(create_renderer()), state(create_state()) {
    initialize();
}
Engine::Engine(ProjectArguments& config)
    : config(config), renderer(create_renderer()), state(create_state()) {
    initialize();
}

Engine::~Engine() {
    SPDLOG_INFO("Destructing engine");
    SPDLOG_TRACE("Engine destructed");
}

void Engine::initialize() {
    SPDLOG_DEBUG("Project configs - render enabled: {}",
                 config.is_render ? "YES" : "NO");
    SPDLOG_DEBUG("Project configs - default map file path: '{}'",
                 config.default_map_file_path);
    SPDLOG_DEBUG("Project configs - auto-save path: '{}'", config.save_path);
    SPDLOG_DEBUG("Project configs - debug graphics enabled: {}",
                 config.is_debug_graphics ? "YES" : "NO");
    SPDLOG_DEBUG("Project configs - walls enabled: {}",
                 config.is_walls_enabled ? "YES" : "NO");

    SPDLOG_DEBUG("Defined globals - COLS: {}, ROWS: {}, NUM_BUTTON_LAYERS: {}",
                 globals::COLS, globals::ROWS, globals::NUM_BUTTON_LAYERS);
    SPDLOG_DEBUG(
        "Defined globals - TEXTBOXHEIGHT: {}, TEXTBOXWIDTH: {}, REGBOXWIDTH: "
        "{}, REGBOXHEIGHT: {}",
        globals::TEXTBOXHEIGHT, globals::TEXTBOXWIDTH, globals::REGBOXWIDTH,
        globals::REGBOXHEIGHT);
}

Renderer* Engine::create_renderer() const {
    return config.is_render ? static_cast<Renderer*>(
                                  new tcodRenderer(config.is_debug_graphics))
                            : static_cast<Renderer*>(new NoneRenderer());
}

EngineState* Engine::create_state() {
    if(!config.replay_play_path.empty()) {
        ant_proto::ReplayHeader header;
        auto status = load_replay_header(config.replay_play_path, header);
        if(status.ok) {
            return new EngineState(config, renderer, &header.environment());
        }
        SPDLOG_ERROR("Failed to load replay header: {}", status.error.message);
    }

    Unpacker unpacker(config.save_path);

    if(unpacker.is_valid()) {
        ant_proto::EngineState msg;
        unpacker >> msg;
        return new EngineState(msg, config, renderer);
    } else {
        return new EngineState(config, renderer);
    }
}

void Engine::update() {
    if(state->is_reload_game) {
        delete state;
        state = create_state();
    }
    state->update();
    // SPDLOG_TRACE("Engine update complete");
}

void Engine::render() {
    state->render();
    renderer->present();

    // SPDLOG_TRACE("Render complete");
}

ReplayStatus Engine::start_replay_recording(const std::string& path) {
    return state->start_replay_recording(path, config);
}

ReplayStatus Engine::start_replay_playback(const std::string& path) {
    return state->start_replay_playback(path);
}

void Engine::stop_replay_recording() { state->stop_replay_recording(); }

bool Engine::replay_has_error() const { return state->replay_has_error(); }

ReplayError Engine::replay_last_error() const {
    return state->replay_last_error();
}

bool Engine::replay_done() const { return state->replay_done(); }

void Engine::action_move_player(long dx, long dy) {
    state->action_move_player(dx, dy);
}

void Engine::action_dig(long dx, long dy) { state->action_dig(dx, dy); }

void Engine::action_create_ant() { state->action_create_ant(); }

void Engine::action_add_program_lines(const std::vector<std::string>& lines) {
    state->action_add_program_lines(lines);
}

void Engine::action_assign_program_to_ant(ulong ant_idx) {
    state->action_assign_program_to_ant(ant_idx);
}

void Engine::action_go_up() { state->action_go_up(); }

void Engine::action_go_down() { state->action_go_down(); }
