#include "hardware/command_config.hpp"
#include "hardware/parse_args.hpp"
#include "hardware/command_parsers.hpp"
#include "hardware/command_compilers.hpp"

#include "spdlog/spdlog.h"

CommandConfig::CommandConfig(
    const std::string &command_string, CommandEnum command_enum,
    std::function<void(ParseArgs &args)> parse,
    std::function<void(DeparseArgs &args)> deparse,
    std::function<void(CompileArgs &args)> compile):
        command_string(command_string),
        command_enum(command_enum),
        parse(parse), deparse(deparse), compile(compile) {
    
        SPDLOG_TRACE("Creating command config for command: {}", command_string);
      }

CommandMap::CommandMap() {
    SPDLOG_TRACE("Creating command map");
    
    // Empty command
    insert(new CommandConfig("NOP", CommandEnum::NOP, NOP_Parser(), NOP_Deparser(), NOP_Compiler()));

    // Load constant command to register
    insert(new CommandConfig("LDI", CommandEnum::LOAD, LoadConstantParser(), LoadConstantDeparser(), LoadConstantCompiler()));

    // Copy register to register
    insert(new CommandConfig("CPY", CommandEnum::COPY, CopyParser(), CopyDeparser(), CopyCompiler()));

    // Add second register to the first
    insert(new CommandConfig("ADD", CommandEnum::ADD, AddParser(), AddDeparser(), AddCompiler()));

    // Subtract second register from the first
    insert(new CommandConfig("SUB", CommandEnum::SUB, SubParser(), SubDeparser(), SubCompiler()));

    // Increment register
    insert(new CommandConfig("INC", CommandEnum::INC, IncParser(), IncDeparser(), IncCompiler()));

    // Decrement register
    insert(new CommandConfig("DEC", CommandEnum::DEC, DecParser(), DecDeparser(), DecCompiler()));

    // MOVE command
    insert(new CommandConfig("MOVE", CommandEnum::MOVE, MoveAntParser(), MoveAntDeparser(), MoveAntCompiler()));
    // JMP command
    insert(new CommandConfig("JMP", CommandEnum::JMP, JumpParser(), JumpDeparser(), JumpCompiler()));

    // JNZ command
    insert(new CommandConfig("JNZ", CommandEnum::JNZ, JumpNotZeroParser(), JumpNotZeroDeparser(), JumpNotZeroCompiler()));
}

CommandMap::~CommandMap() {
    for (auto& pair : enum_map) {
        delete pair.second; // Deleting the pointer
        pair.second = nullptr; // Resetting the pointer to nullptr
    }
    enum_map.clear();
    str_map.clear();
}

void CommandMap::insert(CommandConfig *config) {
    // SPDLOG_TRACE("Inserting command config for command: {}", config->command_string);
    enum_map[config->command_enum] = config;
    str_map[config->command_string] = config;
}

CommandConfig& CommandMap::operator[](CommandEnum command_enum) {
    return *enum_map[command_enum];
}

CommandConfig& CommandMap::operator[](std::string const& command_string) {
    return *str_map[command_string];
}

CommandConfig const& CommandMap::at(CommandEnum command_enum) const {
    return *enum_map.at(command_enum);
}

CommandConfig const& CommandMap::at(std::string const& command_string) const {
    return *str_map.at(command_string);
}

CommandMap::EnumMap::const_iterator CommandMap::find(CommandEnum command) const {
    return enum_map.find(command);
}

CommandMap::EnumMap::const_iterator CommandMap::enum_end() const {
    return enum_map.end();
}

CommandMap::StrMap::const_iterator CommandMap::find(std::string const& s) const {
    return str_map.find(s);
}

CommandMap::StrMap::const_iterator CommandMap::str_end() const {
    return str_map.end();
}
