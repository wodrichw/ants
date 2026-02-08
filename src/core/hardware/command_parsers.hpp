struct ParseArgs;
struct DeparseArgs;
struct CommandConfig;

// We apologize to all future developers for what you are about to see.
// - Kevin 01/18/2024
#define DEFINE_PARSER(T)                                     \
    struct T {                                               \
        void operator()(CommandConfig const &, ParseArgs &); \
    }

DEFINE_PARSER(NoArgCommandParser);
DEFINE_PARSER(LoadConstantParser);
DEFINE_PARSER(TwoRegisterCommandParser);
DEFINE_PARSER(OneRegisterCommandParser);
DEFINE_PARSER(JumpParser);
DEFINE_PARSER(TurnLeftParser);
DEFINE_PARSER(PopParser);
DEFINE_PARSER(PushParser);
DEFINE_PARSER(TurnRightParser);
DEFINE_PARSER(ReturnParser);
DEFINE_PARSER(OneScentCommandParser);
DEFINE_PARSER(SetScentPriorityParser);

#define DEFINE_DEPARSER(T)                                         \
    struct T {                                                     \
        void operator()(CommandConfig const &, DeparseArgs &args); \
    }

DEFINE_DEPARSER(NoArgCommandDeparser);
DEFINE_DEPARSER(LoadConstantDeparser);
DEFINE_DEPARSER(TwoLetterCommandDeparser);
DEFINE_DEPARSER(OneLetterCommandDeparser);
DEFINE_DEPARSER(JumpDeparser);
DEFINE_DEPARSER(TurnLeftDeparser);
DEFINE_DEPARSER(PopDeparser);
DEFINE_DEPARSER(PushDeparser);
DEFINE_DEPARSER(TurnRightDeparser);
DEFINE_DEPARSER(ReturnDeparser);
DEFINE_DEPARSER(SetScentPriorityDeparser);