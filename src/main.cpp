#include "engine.hpp"

int main() {
    Engine engine;
    while ( 1 ) {
        engine.update();
        engine.render();
    }
    return 0;
}
