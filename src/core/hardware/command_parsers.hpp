struct ParseArgs;
struct DeparseArgs;

// We apologize to all future developers for what you are about to see.
// - Kevin 01/18/2024
#define DEFINE_PARSER(T)                   \
    struct T {                             \
        void operator()(ParseArgs &args); \
    } \

DEFINE_PARSER(NOP_Parser);
DEFINE_PARSER(LoadConstantParser);
DEFINE_PARSER(CopyParser);
DEFINE_PARSER(AddParser);
DEFINE_PARSER(SubParser);
DEFINE_PARSER(IncParser);
DEFINE_PARSER(DecParser);
DEFINE_PARSER(MoveAntParser);
DEFINE_PARSER(DigAntParser);
DEFINE_PARSER(JumpParser);
DEFINE_PARSER(JumpNotZeroParser);

#define DEFINE_DEPARSER(T)                   \
    struct T {                             \
        void operator()(DeparseArgs &args); \
    } \

DEFINE_DEPARSER(NOP_Deparser);
DEFINE_DEPARSER(LoadConstantDeparser);
DEFINE_DEPARSER(CopyDeparser);
DEFINE_DEPARSER(AddDeparser);
DEFINE_DEPARSER(SubDeparser);
DEFINE_DEPARSER(IncDeparser);
DEFINE_DEPARSER(DecDeparser);
DEFINE_DEPARSER(MoveAntDeparser);
DEFINE_DEPARSER(DigAntDeparser);
DEFINE_DEPARSER(JumpDeparser);
DEFINE_DEPARSER(JumpNotZeroDeparser);