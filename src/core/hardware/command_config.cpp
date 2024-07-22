#include "hardware/command_config.hpp"
#include "hardware/parse_args.hpp"
#include "hardware/command_parsers.hpp"
#include "hardware/command_compilers.hpp"
#include "hardware/op_def.hpp"

#include "spdlog/spdlog.h"

CommandConfig::CommandConfig(
    const std::string &command_string, CommandEnum command_enum,
    std::function<void(CommandConfig const&, ParseArgs&)> parse,
    std::function<void(CommandConfig const&, DeparseArgs&)> deparse,
    std::function<void(CommandConfig const&, CompileArgs&)> compile):
        command_string(command_string),
        command_enum(command_enum),
        parse(parse), deparse(deparse), compile(compile) {
    
        SPDLOG_TRACE("Creating command config for command: {}", command_string);
      }

CommandMap::CommandMap() {
    SPDLOG_TRACE("Creating command map");
    
    // Empty command
    insert(new CommandConfig("NOP", CommandEnum::NOP, NoArgCommandParser(), NoArgCommandDeparser(), NoArgCommandCompiler<NoOP, 1>()));

    // Load constant command to register
    insert(new CommandConfig("LOAD", CommandEnum::LOAD, LoadConstantParser(), LoadConstantDeparser(), LoadConstantCompiler<LoadConstantOp>()));

    // Copy register to register
    insert(new CommandConfig("COPY", CommandEnum::COPY, TwoRegisterCommandParser(), TwoRegisterCommandDeparser(), TwoRegisterCommandCompiler<CopyOp>()));

    // Add second register to the first
    insert(new CommandConfig("ADD", CommandEnum::ADD, TwoRegisterCommandParser(), TwoRegisterCommandDeparser(), TwoRegisterCommandCompiler<AddOp>()));

    // Subtract second register from the first
    insert(new CommandConfig("SUB", CommandEnum::SUB, TwoRegisterCommandParser(), TwoRegisterCommandDeparser(), TwoRegisterCommandCompiler<SubOp>()));

    // Increment register
    insert(new CommandConfig("INC", CommandEnum::INC, OneRegisterCommandParser(), OneRegisterCommandDeparser(), OneRegisterCommandCompiler<IncOp>()));

    // Decrement register
    insert(new CommandConfig("DEC", CommandEnum::DEC, OneRegisterCommandParser(), OneRegisterCommandDeparser(), OneRegisterCommandCompiler<DecOp>()));

    // MOVE command
    insert(new CommandConfig("MOVE", CommandEnum::MOVE, NoArgCommandParser(), NoArgCommandDeparser(), MoveAntCompiler<MoveOp>()));

    // DIG command
    insert(new CommandConfig("DIG", CommandEnum::DIG, NoArgCommandParser(), NoArgCommandDeparser(), DigAntCompiler<DigOp>()));
        
    // JMP command
    insert(new CommandConfig("JMP", CommandEnum::JMP, JumpParser(), JumpDeparser(), JumpCompiler<JmpOp>()));
 
    // JNZ command
    insert(new CommandConfig("JNZ", CommandEnum::JNZ, JumpParser(), JumpDeparser(), JumpCompiler<JnzOp>()));

    // JNF command
    insert(new CommandConfig("JNF", CommandEnum::JNF, JumpParser(), JumpDeparser(), JumpCompiler<JnfOp>()));

    // CALL command
    insert(new CommandConfig("CALL", CommandEnum::CALL, JumpParser(), JumpDeparser(), JumpCompiler<CallOp>()));

    // LEFT command
    insert(new CommandConfig("LT", CommandEnum::LT, NoArgCommandParser(), NoArgCommandDeparser(), NoArgCommandCompiler<TurnLeftOp>()));

    // POP command
    insert(new CommandConfig("POP", CommandEnum::POP, OneRegisterCommandParser(), OneRegisterCommandDeparser(), OneRegisterCommandCompiler<PopOp>()));

    // PUSH command
    insert(new CommandConfig("PUSH", CommandEnum::PUSH, OneRegisterCommandParser(), OneRegisterCommandDeparser(), OneRegisterCommandCompiler<PushOp>()));

    // RIGHT command
    insert(new CommandConfig("RT", CommandEnum::RT, NoArgCommandParser(), NoArgCommandDeparser(), NoArgCommandCompiler<TurnRightOp>()));

    // RETURN command
    insert(new CommandConfig("RET", CommandEnum::RET, NoArgCommandParser(), NoArgCommandDeparser(), NoArgCommandCompiler<ReturnOp>()));
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
