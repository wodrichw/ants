#include <unordered_set>
#include "controller.hpp"

Worker_Controller::Worker_Controller(
    ParserCommandsAssembler& commands_assember,
    std::vector<std::string>& program_code
):
    parser(
        commands_assember,
        {
            Parser::Command::MOVE,
            Parser::Command::JMP,
            Parser::Command::LOAD,
            Parser::Command::NOP,
        },
        ant_interactor,
        operations,
        program_code
    )
{}

Worker_Controller::~Worker_Controller() {}


void Worker_Controller::handleClockPulse()
{
    operations.handleClockPulse();
}
