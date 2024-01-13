#include <cstdlib>
#include <ios>
#include <sstream>

#include "operations.hpp"
#include "parser.hpp"

EngineInteractor::EngineInteractor(const EngineInteractor& rhs):
    move_ant(rhs.move_ant),
    status()
{
    status.p_err = rhs.status.p_err;
    status.err_msg = rhs.status.err_msg;
}

Parser::Parser(
    ParserCommandsAssembler& commands_assember,
    std::unordered_set<Command> command_set,
    EngineInteractor& interactor,
    Operations& operations,
    std::vector<std::string>& program_code
): 
    commands()
{
    for( auto& command: command_set ) {
        commands.insert({commands_assember[command].command_string, commands_assember[command].assemble(interactor, operations)});
    }

    parse(interactor, operations, program_code);
}

void Parser::parse(EngineInteractor& interactor, Operations& operations, std::vector<std::string>& program_code)
{
    std::unordered_map<std::string, size_t> label_map;
    bool empty_program = true;
    size_t op_idx;
    for (std::string& line: program_code) {
        std::istringstream word_stream(line);
        bool comment = false;
        do {
            std::string word;
            word_stream >> word;

            if( !word_stream ) continue; // no need to read an empty line
            if( word[0] == '#' ) { // dont read comments
                comment = true;
                continue;
            }
            empty_program = false;

            if( word[word.length() - 1] == ':' ) { // handle label
                if( word.length() == 1 ) {
                    interactor.status.error("DEFINED AN EMPTY LABEL");
                    return;
                }
                std::string label(word.substr(0, word.length()-2));
                operations.add_label({label, op_idx});
                continue;
            }

            auto c = commands.find(word);
            if (c == commands.end()) {
                std::stringstream error_stream;
                error_stream << "THE FOLLOWING IS NOT A VALID COMMAND: " << word;
                interactor.status.error(error_stream.str());
                return;
            }
            c->second.parse(word_stream); // parse command
            if( interactor.status.p_err ) return; // return if error parsing command
            ++op_idx;
        } while( word_stream && !comment );
    }
    if ( empty_program ) {
        interactor.status.error("WILL NOT ADD AN ANT WITHOUT A PROGRAM");
    }
}

ParserCommandsAssembler::ParserCommandsAssembler(): _map()
{
    // MOVE command
    insert(new Parser::CommandConfig(
        "MOVE",
        Parser::Command::MOVE,
        Parser::parse_move
    ));

    // GOTO command
    insert(new Parser::CommandConfig(
        "GOTO",
        Parser::Command::GOTO,
        Parser::parse_goto
    ));
}


/************************************************************************
 ********************** COMMAND PARSING FUNCTIONS ***********************
 ************************************************************************/
Parser::CommandParser Parser::parse_move(EngineInteractor& interactor, Operations& operations)
{
    return Parser::CommandParser(1, //TODO: remove num args or handle it automatically
    [&interactor, &operations](std::istringstream& arg_sstream) {
        std::string word;
        arg_sstream >> word;
        if( !arg_sstream ) {
            interactor.status.error("NEED DIRECTION DEFINED FOR MOVE COMMAND");
            return;
        }
        int dx = 0, dy = 0;
        if(        word == "UP" ) {
            dy = -1;
        } else if( word == "LEFT" ) {
            dx = -1;
        } else if( word == "DOWN" ) {
            dy = 1;
        } else if( word == "RIGHT" ) {
            dx = 1;
        } else {
            interactor.status.error("TODO: MAKE ERRORS MAKE SENSE1");
            return;
        }
        operations.add_op(MoveOp(interactor, dx, dy));

        arg_sstream >> word;
        if ( arg_sstream && word[0] != '#' ) {
            interactor.status.error("TODO: MAKE ERRORS MAKE SENSE");
        }
    });
}

Parser::CommandParser Parser::parse_goto(EngineInteractor& interactor, Operations& operations)
{
    return Parser::CommandParser(1,
    [&interactor, &operations](std::istringstream& arg_sstream) {
        std::string word;
        std::string address;
        arg_sstream >> address;
        if( !arg_sstream ) {
            interactor.status.error("NEED DIRECTION DEFINED FOR GOTO COMMAND");
            return;
        }
        bool is_op_idx = std::find_if(address.begin(), address.end(), [](unsigned char c) {
                return !std::isdigit(c);
        }) == address.end();

        arg_sstream >> word;
        if( !arg_sstream.eof() ) {
            interactor.status.error("GOTO TAKES ONLY ONE ARGUMENT");
            return;
        }
        if( is_op_idx ) {
            operations.add_op(GotoOp(std::strtoul(address.c_str(), nullptr, 10), operations));
        } else {
            operations.add_op(GotoOp(address, operations));
        }
    });
}

