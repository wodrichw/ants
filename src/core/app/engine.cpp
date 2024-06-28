#include "app/engine.hpp"
#include "app/engine_state.hpp"
#include "app/arg_parse.hpp"
#include "ui/render.hpp"
#include "spdlog/spdlog.h"

Engine::Engine()
    : config(0, nullptr), unpacker(config.save_path),
      renderer(create_renderer()), state(create_state()) {
    initialize();
}
Engine::Engine(int argc, char* argv[])
    : config(argc, argv), unpacker(config.save_path),
      renderer(create_renderer()), state(create_state()) {
    initialize();
}
Engine::Engine(ProjectArguments& config)
    : config(config),
      unpacker(config.save_path), renderer(create_renderer()),
      state(create_state()) {
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
    return config.is_render ? static_cast<Renderer*>(new tcodRenderer(
        config.is_debug_graphics)) : static_cast<Renderer*>(new NoneRenderer());
}

EngineState* Engine::create_state() {
    return unpacker.is_valid() ? new EngineState(unpacker, config, renderer) : new EngineState(config, renderer);
}

void Engine::update() {
    state->update();

    if (state->is_reload_game) {
        delete state;
        state = new EngineState(config, renderer);
    }
    // SPDLOG_TRACE("Engine update complete");
}

void Engine::render() {
    state->render();
    renderer->present();

    // SPDLOG_TRACE("Render complete");
}
