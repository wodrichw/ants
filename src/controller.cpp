#include <unordered_set>
#include "controller.hpp"


Worker_Controller::Worker_Controller(
    ParserCommandsAssembler& commands_assember,
    EngineInteractor& interactor,
    std::vector<std::string>& program_code
):
    operations(),
    parser(
        commands_assember,
        {
            Parser::Command::MOVE,
            Parser::Command::JMP
        },
        interactor,
        operations,
        program_code
    )
{}

Worker_Controller::~Worker_Controller() {}


void Worker_Controller::handleClockPulse()
{
    if( operations.op_idx == operations.size() ) return; // don't do anything if out of ops
    operations[operations.op_idx]();
    if( operations.jmp_set ) {
        operations.jmp_set = false;
    } else {
        ++operations.op_idx;
    }
}


