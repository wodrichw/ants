#include "app/facade.hpp"

#include <SDL_timer.h>

AntGameFacade::AntGameFacade() : config(0, nullptr), unpacker(config.save_path), engine(unpacker.is_valid() ? Engine(unpacker, config) : Engine(config)), clock_timeout(SDL_GetTicks64()) {
    initialize();
}
AntGameFacade::AntGameFacade(int argc, char* argv[])
    : config(argc, argv), unpacker(config.save_path), engine(unpacker.is_valid() ? Engine(unpacker, config) : Engine(config)), clock_timeout(SDL_GetTicks64()) {
    initialize();
}
AntGameFacade::AntGameFacade(ProjectArguments& config) : config(config), unpacker(config.save_path), engine(unpacker.is_valid() ? Engine(unpacker, config) : Engine(config)), clock_timeout(SDL_GetTicks64()) {
    initialize();
}

bool AntGameFacade::update() {
    if(clock_timeout >= SDL_GetTicks64()) return false;
    clock_timeout += 17; // 1000ms / 60 FPS = 17

    engine.update();
    engine.render();
    return true;
}

void AntGameFacade::engine_update() {
    engine.update();
}

void AntGameFacade::initialize() {
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
