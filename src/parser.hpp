#pragma once

#include <functional>
#include <string>
#include <unordered_set>
#include <sstream>

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
    void direction(std::istringstream &ss, int &dx, int &dy,
                    ParserStatus &status);

    // Check that no more arguments exist to be parsed.
    void terminate(std::istringstream &ss, ParserStatus &status,
                    const std::string &err_msg);
}

struct ParserArgs {
    EngineInteractor& engine_interactor;
    BrainInteractor& brain_interactor;
    Operations& operations;
    std::istringstream& code_stream;
};

class ParserCommandsAssembler;

class Parser {
public:
    enum Command { 
        JMP,
        LOAD,
        MOVE,
        NOP
    };

  struct CommandConfig {
    std::string command_string;
    Command command_enum;
    std::function<void(ParserArgs& args)> assemble;
    CommandConfig(const std::string &command_string, Command command_enum,
                  std::function<void(ParserArgs &args)> assemble);
  };

private:
  void parse(EngineInteractor& engine_interactor, BrainInteractor& brain_interactor,
    Operations& operations, std::vector<std::string>& program_code);

public:

  std::unordered_map<std::string, std::function<void(ParserArgs& args)>> commands;
  Parser(ParserCommandsAssembler &commands_assember,
         std::unordered_set<Command> command_set, EngineInteractor &interactor,
         Operations &operations, std::vector<std::string> &program_code);
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
#define DEFINE_PARSER(T) struct T { \
    void operator()(ParserArgs &args); \
}

DEFINE_PARSER(NOP_Parser);
DEFINE_PARSER(LoadConstantParser);
DEFINE_PARSER(MoveAntParser);
DEFINE_PARSER(JumpParser);
