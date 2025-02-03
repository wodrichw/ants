#pragma once

#include "app/arg_parse.hpp"

class Renderer;
struct EngineState;

class Engine {
    ProjectArguments config;
    Renderer* renderer = {};
    EngineState* state = {};

   public:
    Engine();
    Engine(int argc, char* argv[]);
    Engine(ProjectArguments& config);
    ~Engine();
    void update();
    void render();

   private:
    void initialize();
    Renderer* create_renderer() const;
    EngineState* create_state();
};
