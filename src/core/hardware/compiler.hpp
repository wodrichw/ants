#pragma once

#include "hardware/command_config.hpp"
#include "hardware/compile_args.hpp"

// This class is not joined with Parser even though they are very similar
// because they are used in different places, where the parser is used with the
// text editor and the compiler is used during worker creation. Outside of that
// there is no significant reason to keep them separate if there is a reason
// to do so.
class Compiler {
   public:
    Compiler(CommandMap const& command_map);

    void compile(CompileArgs& args);

    CommandMap const& command_map;
};
