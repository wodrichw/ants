#include "hardware/command_config.hpp"
#include "hardware/parse_args.hpp"
#include "hardware/command_parsers.hpp"

#include "spdlog/spdlog.h"

CommandConfig::CommandConfig(
    const std::string &command_string, Command command_enum,
    std::function<void(ParseLineArgs &args)> assemble)
    : command_string(command_string),
      command_enum(command_enum),
      assemble(assemble) {
        SPDLOG_TRACE("Creating command config for command: {}", command_string);
      }

void insert(CommandMap& map, CommandConfig *config) {
    // SPDLOG_TRACE("Inserting command config for command: {}", config->command_string);
    map[config->command_enum] = config;
}

void get_command_map(CommandMap& map) {
    SPDLOG_TRACE("Creating command assembler");
    
    // Empty command
    insert(map,
        new CommandConfig("NOP", Command::NOP, NOP_Parser()));

    // Load constant command to register
    insert(map, new CommandConfig("LDI", Command::LOAD,
                                     LoadConstantParser()));

    // Copy register to register
    insert(map,
        new CommandConfig("CPY", Command::COPY, CopyParser()));

    // Add second register to the first
    insert(map, new CommandConfig("ADD", Command::ADD, AddParser()));

    // Subtract second register from the first
    insert(map, new CommandConfig("SUB", Command::SUB, SubParser()));

    // Increment register
    insert(map, new CommandConfig("INC", Command::INC, IncParser()));

    // Decrement register
    insert(map, new CommandConfig("DEC", Command::DEC, DecParser()));

    // MOVE command
    insert(map, new CommandConfig("MOVE", Command::MOVE,
                                     MoveAntParser()));

    // JMP command
    insert(map,
        new CommandConfig("JMP", Command::JMP, JumpParser()));

    // JNZ command
    insert(map, new CommandConfig("JNZ", Command::JNZ,
                                     JumpNotZeroParser()));
}
