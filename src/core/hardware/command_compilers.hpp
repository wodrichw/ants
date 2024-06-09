
struct CompileArgs;

// I apologize again to all future developers for what you are about to see.
// - Kevin 05/04/2024
#define DEFINE_COMPILER(T)                   \
    struct T {                             \
        void operator()(CompileArgs &parse_line); \
    }

DEFINE_COMPILER(NOP_Compiler);
DEFINE_COMPILER(LoadConstantCompiler);
DEFINE_COMPILER(CopyCompiler);
DEFINE_COMPILER(AddCompiler);
DEFINE_COMPILER(SubCompiler);
DEFINE_COMPILER(IncCompiler);
DEFINE_COMPILER(DecCompiler);
DEFINE_COMPILER(MoveAntCompiler);
DEFINE_COMPILER(JumpCompiler);
DEFINE_COMPILER(JumpNotZeroCompiler);
