#include <unordered_set>

#include "controller.hpp"
#include "spdlog/spdlog.h"

Worker_Controller::Worker_Controller(ParserCommandsAssembler& commands_assember,
                                     std::vector<std::string>& program_code)
    : parser(commands_assember,
             {Parser::Command::ADD, Parser::Command::DEC, Parser::Command::INC,
              Parser::Command::JMP, Parser::Command::JNZ, Parser::Command::LOAD,
              Parser::Command::MOVE, Parser::Command::NOP, Parser::Command::SUB,
              Parser::Command::COPY},
             ant_interactor, operations, program_code) {
                SPDLOG_DEBUG("Worker Controller created");
             }

Worker_Controller::~Worker_Controller() {
    SPDLOG_DEBUG("Destructing Worker Controller");
}

void Worker_Controller::handleClockPulse() {
    SPDLOG_DEBUG("Handling clock pulse for worker controller");
    operations.handleClockPulse();
}
