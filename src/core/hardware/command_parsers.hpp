struct ParseLineArgs;

// We apologize to all future developers for what you are about to see.
// - Kevin 01/18/2024
#define DEFINE_PARSER(T)                   \
    struct T {                             \
        void operator()(ParseLineArgs &parse_line); \
    }

DEFINE_PARSER(NOP_Parser);
DEFINE_PARSER(LoadConstantParser);
DEFINE_PARSER(CopyParser);
DEFINE_PARSER(AddParser);
DEFINE_PARSER(SubParser);
DEFINE_PARSER(IncParser);
DEFINE_PARSER(DecParser);
DEFINE_PARSER(MoveAntParser);
DEFINE_PARSER(JumpParser);
DEFINE_PARSER(JumpNotZeroParser);