#pragma once

#include "app/arg_parse.hpp"
#include "app/engine.hpp"

class AntGameFacade {

    public:
    AntGameFacade();
    AntGameFacade(int argc, char* argv[]);

    void update();

    private:
    void initialize();

    ProjectArguments config;
    Unpacker unpacker;
    Engine engine;
};