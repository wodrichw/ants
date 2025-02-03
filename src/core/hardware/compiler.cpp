#include "hardware/compiler.hpp"

Compiler::Compiler(CommandMap const& command_map) : command_map(command_map) {}

void Compiler::compile(CompileArgs& args) {
    while(args.code_it != args.code.end()) {
        CommandEnum instruction = static_cast<CommandEnum>(*args.code_it >> 3);
        CommandConfig const& command = command_map.at(instruction);
        command.compile(command, args);
    }
}
