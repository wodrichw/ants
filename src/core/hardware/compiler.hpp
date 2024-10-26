#pragma once

#include "hardware/command_config.hpp"
#include "hardware/machine_code.hpp"
#include "hardware/compile_args.hpp"

// This class is not joined with Parser even though they are very similar
// because they are used in different places, where the parser is used with the
// text editor and the compiler is used during worker creation. Outside of that
// there is no significant reason to keep them separate if there is a reason
// to do so.
class Compiler {
    public:

    Compiler(CommandMap const& command_map) : command_map(command_map) {}

    void compile(CompileArgs& args) {
        while(args.code_it != args.code.end()) {
            CommandEnum instruction = static_cast<CommandEnum>(*args.code_it >> 3);
            CommandConfig const& command = command_map.at(instruction);
            command.compile(command, args);
        }
    }

    CommandMap const& command_map;
};
