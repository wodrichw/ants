#pragma once

#include "app/arg_parse.hpp"
#include "app/engine.hpp"

class AntGameFacade {

    public:
    AntGameFacade();
    AntGameFacade(int argc, char* argv[]);
    AntGameFacade(ProjectArguments& config);

    bool update();
    void engine_update();

    private:
    void initialize();

    ProjectArguments config;
    Unpacker unpacker;
    Engine engine;
    ulong clock_timeout;
};