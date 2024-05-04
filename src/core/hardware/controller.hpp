#pragma once

#include <string>
#include <vector>

#include "hardware/parse_args.hpp"
#include "hardware/parser.hpp"
#include "hardware/operations.hpp"

class ClockController {
   public:
    virtual void handleClockPulse() = 0;
    virtual ~ClockController() = default;
};

class Worker_Controller : public ClockController {
   public:
    Operations operations;
    ParserArgs parser_args;
    Parser parser;

    Worker_Controller(
        CommandMap& command_map,
        DualRegisters& registers, MapEntity& entity, Map& map,
        std::vector<std::string>& program_code);
    ~Worker_Controller();

    void handleClockPulse() override;
};
