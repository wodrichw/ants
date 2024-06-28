#pragma once

#include "app/engine.hpp"

using ulong = unsigned long;

class AntGameFacade {

    public:
    AntGameFacade();
    AntGameFacade(int argc, char* argv[]);
    AntGameFacade(ProjectArguments& config);

    bool update();
    void engine_update();

    private:

    Engine engine;
    ulong clock_timeout;
};