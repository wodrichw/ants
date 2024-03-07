#include "controller.hpp"

#include <unordered_set>

Worker_Controller::Worker_Controller(ParserCommandsAssembler& commands_assember,
                                     std::vector<std::string>& program_code)
    : parser(commands_assember,
             {Parser::Command::ADD, Parser::Command::DEC, Parser::Command::INC,
              Parser::Command::JMP, Parser::Command::JNZ, Parser::Command::LOAD,
              Parser::Command::MOVE, Parser::Command::NOP, Parser::Command::SUB,
              Parser::Command::COPY},
             ant_interactor, operations, program_code) {}

Worker_Controller::~Worker_Controller() {}

void Worker_Controller::handleClockPulse() { operations.handleClockPulse(); }
