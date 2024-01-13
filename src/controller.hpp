#pragma once

#include <vector>
#include <string>
#include <functional>
#include <unordered_set>
#include <unordered_map>

#include "operations.hpp"
#include "parser.hpp"


class ClockController {
public:
    virtual void handleClockPulse() = 0;
    virtual ~ClockController() = default;
};


class  Worker_Controller: public ClockController {
public:
    struct Brain {
        int acc_reg;
        int bak_reg;
    };

    Brain* brain;

    Operations operations;
    Parser parser;

    Worker_Controller(ParserCommandsAssembler& commands_assember, EngineInteractor& interactor, std::vector<std::string>& program_code);
    ~Worker_Controller();

    void handleClockPulse() override;
};
