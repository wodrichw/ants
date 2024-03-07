#include <SDL_timer.h>

#include "engine.hpp"

// TODO: pull in the tcod clock to notify when to update worker ants
// This clock speed shoud be able to be set by an interface rendered
// on the screen.
int main() {
    Engine engine;
    while(1) {
        SDL_GetTicks();

        engine.update();
        engine.render();
    }
    return 0;
}
