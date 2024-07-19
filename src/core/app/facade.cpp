#include "app/facade.hpp"

#include <SDL_timer.h>

AntGameFacade::AntGameFacade() : engine(), clock_timeout(SDL_GetTicks64()) {}
AntGameFacade::AntGameFacade(int argc, char* argv[])
    : engine(argc, argv), clock_timeout(SDL_GetTicks64()) {}
AntGameFacade::AntGameFacade(ProjectArguments& config) : engine(config), clock_timeout(SDL_GetTicks64()) {}

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
