#pragma once

#include <functional>
#include <sstream>
#include <string>
#include <unordered_set>

#include "ant_interactor.hpp"
#include "globals.hpp"
#include "operations.hpp"

// Parser helpers
namespace TokenParser {
    // Parse string to integer
    cpu_word_size integer(std::istringstream &ss);

    // Parse register names to an index
    // Register names are single characters A-Z
    // A -> 0
    // B -> 1
    // ...
    cpu_word_size register_idx(std::istringstream &ss);

    // Parse direction keyword to a dx and dy pair
    void direction(std::istringstream &ss, long &dx, long &dy,
                   ParserStatus &status);

    void get_label_address(std::istringstream &ss, std::string &address,
                           bool &is_op_idx, ParserStatus &status);

    // Check that no more arguments exist to be parsed.
    void terminate(std::istringstream &ss, ParserStatus &status,
                   const std::string &err_msg);
}  // namespace TokenParser

struct ParserArgs {
    AntInteractor &ant_interactor;
    Operations &operations;
    std::istringstream &code_stream;
    ParserStatus &status;
};

class ParserCommandsAssembler;

class Parser {
   public:
    enum Command { ADD, COPY, DEC, INC, JNZ, JMP, LOAD, MOVE, NOP, SUB };

    struct CommandConfig {
        std::string command_string;
        Command command_enum;
        std::function<void(ParserArgs &args)> assemble;
        CommandConfig(const std::string &command_string, Command command_enum,
                      std::function<void(ParserArgs &args)> assemble);
    };
    ParserStatus status;

   private:
    void parse(AntInteractor &ant_interactor, Operations &operations,
               std::vector<std::string> &program_code);
    bool handle_label(Operations &operations, std::string const &word);

   public:
    std::unordered_map<std::string, std::function<void(ParserArgs &args)>>
        commands;
    Parser(ParserCommandsAssembler &commands_assember,
           std::unordered_set<Command> command_set,
           AntInteractor &ant_interactor, Operations &operations,
           std::vector<std::string> &program_code);
};

class ParserCommandsAssembler {
    std::unordered_map<Parser::Command, Parser::CommandConfig *> _map;

   public:
    ParserCommandsAssembler();
    const Parser::CommandConfig &operator[](Parser::Command command);
    void insert(Parser::CommandConfig *config);
};

// We apologize to all future developers for what you are about to see.
// - Kevin 01/18/2024
#define DEFINE_PARSER(T)                   \
    struct T {                             \
        void operator()(ParserArgs &args); \
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
