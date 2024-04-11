#include "facade.hpp"

// TODO: pull in the tcod clock to notify when to update worker ants
// This clock speed shoud be able to be set by an interface rendered
// on the screen.

int main(int argc, char* argv[]) {
    AntGameFacade core(argc, argv);
    while(1) {
        core.update();
    }
    return 0;
}
