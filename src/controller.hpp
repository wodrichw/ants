#pragma once

#include <vector>
#include <string>

#include "parser.hpp"

class ClockController {
public:
    virtual void handleClockPulse() = 0;
    virtual ~ClockController() = default;
};


class  Worker_Controller: public ClockController {
public:
    AntInteractor ant_interactor;
    Operations operations;
    Parser parser;

    Worker_Controller(ParserCommandsAssembler& commands_assember,std::vector<std::string>& program_code);
    ~Worker_Controller();

    void handleClockPulse() override;
};
