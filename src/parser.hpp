#pragma once

#include <string>
#include <functional>
#include <unordered_set>

#include "operations.hpp"


class ParserCommandsAssembler;

class Parser {
public:
    enum Command {
        MOVE,
        GOTO
    };

    struct CommandParser {
        size_t num_args;
        std::function<void(std::istringstream& arg_sstream)> parse;
        CommandParser(size_t num_args, std::function<void(std::istringstream& arg_sstream)> parse):
            num_args(num_args), parse(parse)
        {}
    };

    struct CommandConfig {
        std::string command_string;
        Command command_enum;
        std::function<CommandParser(EngineInteractor& interactor, Operations& operations)> assemble;
        CommandConfig(
            const std::string& command_string,
            Command command_enum, 
            std::function<CommandParser(EngineInteractor& interactor, Operations&)> assemble
        ):
            command_string(command_string),
            command_enum(command_enum),
            assemble(assemble)
        {}
    };

private:
    void parse(EngineInteractor& interactor, Operations& operations, std::vector<std::string>& program_code);
public:
    // Methods to parse commands
    static CommandParser parse_move(EngineInteractor& interactor, Operations& operations);
    static CommandParser parse_goto(EngineInteractor& interactor, Operations& operations);

    std::unordered_map<std::string, CommandParser> commands;
    Parser(ParserCommandsAssembler& commands_assember,
        std::unordered_set<Command> command_set,
        EngineInteractor& interactor,
        Operations& operations,
        std::vector<std::string>& program_code
    );
};

class ParserCommandsAssembler {
    std::unordered_map<Parser::Command, Parser::CommandConfig*> _map;

public:
    ParserCommandsAssembler();
    const Parser::CommandConfig& operator[](Parser::Command command) { return *_map[command]; }
    void insert(Parser::CommandConfig* config) { _map[config->command_enum] = config; }
};

