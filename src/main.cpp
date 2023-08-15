#include <libtcod.hpp>

#include "engine.hpp"


int main() {
    Engine engine;
    // while ( !TCODConsole::isWindowClosed() ) {
    while ( 1 ) {
        engine.update();
        engine.render();
        // TCODConsole::flush();
    }
    return 0;
}
