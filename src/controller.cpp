#include <sstream>
#include <unordered_set>
#include "controller.hpp"

EngineInteractor::EngineInteractor(const EngineInteractor& rhs):
    move_ant(rhs.move_ant),
    status()
{
    status.p_err = rhs.status.p_err;
    status.err_msg = rhs.status.err_msg;
}

ParserCommandsAssembler::ParserCommandsAssembler(): _map()
{
    // MOVE command
    insert(new Parser::CommandConfig(
        "MOVE",
        Parser::Command::MOVE,
        [](EngineInteractor& interactor, std::vector<Controller::Op>& operations){
            return Parser::CommandParser(1, //TODO: remove num args or handle it automatically
                [&interactor, &operations](std::istringstream& arg_sstream) {
                    std::string word;
                    arg_sstream >> word;
                    int dx = 0, dy = 0;
                    if        ( word == "UP" ) {
                        dy = -1;
                    } else if ( word == "LEFT" ) {
                        dx = -1;
                    } else if ( word == "DOWN" ) {
                        dy = 1;
                    } else if ( word == "RIGHT" ) {
                        dx = 1;
                    } else {
                        interactor.status.error("TODO: MAKE ERRORS MAKE SENSE");
                        return;
                    }
                    operations.push_back(Parser::Move(interactor, dx, dy));

                    arg_sstream >> word;
                    if ( arg_sstream ) {
                        interactor.status.error("TODO: MAKE ERRORS MAKE SENSE");
                    }
                }
            );
        }
    ));
}

Parser::Move::Move(const EngineInteractor& interactor, int dx, int dy): interactor(interactor), dx(dx), dy(dy) {}

void Parser::Move::operator()() {
    interactor.move_ant(dx, dy);
}
Parser::Parser(
    ParserCommandsAssembler& commands_assember,
    std::unordered_set<Command> command_set,
    EngineInteractor& interactor,
    std::vector<Controller::Op>& operations,
    std::vector<std::string>& program_code
    ) : commands()
{
    for( auto& command: command_set ) {
        commands.insert({commands_assember[command].command_string, commands_assember[command].assemble(interactor, operations)});
    }

    bool empty_program = true;
    for (std::string& line: program_code) {
        std::istringstream word_stream(line);
        do {
            std::string word;
            word_stream >> word;

            if ( !word_stream ) continue; // no need to read an empty line
            empty_program = false;

            auto c = commands.find(word);
            if (c == commands.end()) return;
            c->second.parse(word_stream);
        } while (word_stream);
    }
    if ( empty_program ) {
        interactor.status.error("WILL NOT ADD AN ANT WITHOUT A PROGRAM");
        return;
    }
}

Worker_Controller::Worker_Controller(ParserCommandsAssembler& commands_assember, EngineInteractor& interactor, std::vector<std::string>& program_code):
    operations(), operation_idx(), parser(commands_assember, {Parser::Command::MOVE}, interactor, operations, program_code)
{}

Worker_Controller::~Worker_Controller() {}


void Worker_Controller::handleClockPulse()
{
    operations[operation_idx]();
    operation_idx == operations.size()-1? operation_idx = 0: operation_idx++;
}
