#include <SDL_timer.h>

#include "arg_parse.hpp"
#include "engine.hpp"
#include "spdlog/spdlog.h"
#include "globals.hpp"

// TODO: pull in the tcod clock to notify when to update worker ants
// This clock speed shoud be able to be set by an interface rendered
// on the screen.


int main(int argc, char* argv[]) {
    ProjectArguments config(argc, argv);
    SPDLOG_DEBUG("Configurations loaded");

    SPDLOG_DEBUG("Project configs: render enabled: {}", config.is_render ? "YES" : "NO");
    SPDLOG_DEBUG("Project configs: default map file path: '{}'", config.default_map_file_path);

    SPDLOG_DEBUG("Defined globals: COLS: {}, ROWS: {}, NUM_BUTTON_LAYERS: {}",
                 globals::COLS, globals::ROWS, globals::NUM_BUTTON_LAYERS);
    SPDLOG_DEBUG("Defined globals: TEXTBOXHEIGHT: {}, TEXTBOXWIDTH: {}, REGBOXWIDTH: {}, REGBOXHEIGHT: {}",
                 globals::TEXTBOXHEIGHT, globals::TEXTBOXWIDTH, globals::REGBOXWIDTH, globals::REGBOXHEIGHT);

    SPDLOG_INFO("Initializing engine");
    Engine engine(config);
    while(1) {
        SDL_GetTicks();

        engine.update();
        engine.render();
    }
    return 0;
}
