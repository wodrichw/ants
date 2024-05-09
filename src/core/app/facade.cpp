#include "app/facade.hpp"

#include <SDL_timer.h>

AntGameFacade::AntGameFacade() : config(0, nullptr), engine(config) {
    initialize();
}
AntGameFacade::AntGameFacade(int argc, char* argv[])
    : config(argc, argv), engine(config) {
    initialize();
}

void AntGameFacade::update() {
    SDL_GetTicks();
    engine.update();
    engine.render();
}

void AntGameFacade::initialize() {
    SPDLOG_DEBUG("Project configs - render enabled: {}",
                 config.is_render ? "YES" : "NO");
    SPDLOG_DEBUG("Project configs - default map file path: '{}'",
                 config.default_map_file_path);
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
