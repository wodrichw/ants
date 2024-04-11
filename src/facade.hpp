#include "arg_parse.hpp"
#include "engine.hpp"

class AntGameFacade {

    public:
    AntGameFacade();
    AntGameFacade(int argc, char* argv[]);

    void update();

    private:
    void initialize();

    ProjectArguments config;
    Engine engine;
};