#include <unordered_set>

#include "hardware/controller.hpp"
#include "spdlog/spdlog.h"

Worker_Controller::Worker_Controller(CommandMap& command_map,
                                    DualRegisters& registers, MapEntity& entity,
                                    Map& map,
                                    std::vector<std::string>& program_code)
    : parser_args(registers, entity, map, operations),
        parser(command_map,
             {Command::ADD, Command::DEC, Command::INC,
              Command::JMP, Command::JNZ, Command::LOAD,
              Command::MOVE, Command::NOP, Command::SUB,
              Command::COPY},
              program_code, parser_args) {
                SPDLOG_DEBUG("Worker Controller created");
             }

Worker_Controller::~Worker_Controller() {
    SPDLOG_DEBUG("Destructing Worker Controller");
}

void Worker_Controller::handleClockPulse() {
    SPDLOG_DEBUG("Handling clock pulse for worker controller");
    operations.handleClockPulse();
}
