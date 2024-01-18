#include <climits>
#include <cstdlib>
#include <ios>
#include <sstream>

#include "operations.hpp"
#include "parser.hpp"

cpu_word_size TokenParser::integer(std::istringstream &ss) {
  std::string word;
  ss >> word;
  return std::stoi(word);
}
cpu_word_size TokenParser::register_idx(std::istringstream &ss) {
  char firstChar;
  ss >> firstChar;

  if (firstChar >= 'A' && firstChar <= 'Z') {
    return firstChar - 'A';
  }

  // Handle the case when the character is not a capital letter
  // You can choose to throw an exception, return a default value, or handle it
  // in a different way based on your requirements. For example, returning a
  // specific value like UINT_MAX to indicate an error.
  return UINT_MAX;
}
void TokenParser::direction(std::istringstream &ss, int &dx, int &dy,
                            ParserStatus &status) {
  std::string word;
  ss >> word;

  if (word == "UP") {
    dy = -1;
  } else if (word == "LEFT") {
    dx = -1;
  } else if (word == "DOWN") {
    dy = 1;
  } else if (word == "RIGHT") {
    dx = 1;
  } else {
    status.error("Invalid direction keyword - acceptable directions are: UP, "
                 "LEFT, DOWN and RIGHT.");
  }
}

void TokenParser::terminate(std::istringstream &ss, ParserStatus &status,
                            const std::string &err_msg) {
  std::string word;
  ss >> word;
  if (ss && word[0] != '#') {
    status.error(err_msg);
  }
}

Parser::Parser(
    ParserCommandsAssembler& commands_assember,
    std::unordered_set<Command> command_set,
    EngineInteractor& engine_interactor,
    Operations& operations,
    std::vector<std::string>& program_code
): 
    commands()
{
    for( auto& command: command_set ) {
        commands.insert({commands_assember[command].command_string, commands_assember[command].assemble});
    }

    BrainInteractor brain_interactor;
    parse(engine_interactor, brain_interactor, operations, program_code);
}

struct CommentHandler {
    bool has_comment = false;
    bool operator()(std::string const& word) {
        if( word[0] != '#' ) return false;
        return has_comment = true;
    }
};

bool handle_label(EngineInteractor& interactor, Operations& operations, std::string const& word) {
    if( word[word.length() - 1] != ':' ) return false;

    if( word.length() == 1 ) {
        interactor.status.error("DEFINED AN EMPTY LABEL");
        return false;
    }
    std::string label(word.substr(0, word.length()-1));
    operations.add_label({label, operations.size()});
    return true;
}

void Parser::parse(EngineInteractor& engine_interactor, BrainInteractor& brain_interactor,
    Operations& operations, std::vector<std::string>& program_code)
{
    std::unordered_map<std::string, size_t> label_map;
    bool empty_program = true;
    for (std::string& line: program_code) {
        std::istringstream word_stream(line);
        ParserArgs args{engine_interactor, brain_interactor, operations, word_stream};
        CommentHandler comment_handler;
        do {
            std::string word;
            word_stream >> word;

            if( !word_stream ) continue; // no need to read an empty line
            if( comment_handler(word) ) continue;

            empty_program = false;

            if (handle_label(engine_interactor, operations, word)) continue;
            if (engine_interactor.status.p_err) return;

            auto c = commands.find(word);
            if (c == commands.end()) {
                std::stringstream error_stream;
                error_stream << "THE FOLLOWING IS NOT A VALID COMMAND: " << word;
                engine_interactor.status.error(error_stream.str());
                return;
            }
            c->second(args); // parse command
            if( engine_interactor.status.p_err ) return; // return if error parsing command
        } while( word_stream && !comment_handler.has_comment );
    }
    if ( empty_program ) {
        engine_interactor.status.error("WILL NOT ADD AN ANT WITHOUT A PROGRAM");
    }
}
const Parser::CommandConfig &
ParserCommandsAssembler::operator[](Parser::Command command) {
  return *_map[command];
}
Parser::CommandConfig::CommandConfig(
    const std::string &command_string, Command command_enum,
    std::function<void(ParserArgs &args)> assemble)
    : command_string(command_string), command_enum(command_enum),
      assemble(assemble) {}


ParserCommandsAssembler::ParserCommandsAssembler(): _map()
{
    // Empty command
    insert(new Parser::CommandConfig(
        "NOP",
        Parser::Command::NOP,
        NOP_Parser()
    ));

    // Load constant command to register
    insert(new Parser::CommandConfig(
        "LOAD",
        Parser::Command::LOAD,
        LoadConstantParser()
    ));

    // MOVE command
    insert(new Parser::CommandConfig(
        "MOVE",
        Parser::Command::MOVE,
        MoveAntParser()
    ));

    // JMP command
    insert(new Parser::CommandConfig(
        "JMP",
        Parser::Command::JMP,
        JumpParser()
    ));
}

void ParserCommandsAssembler::insert(Parser::CommandConfig *config) {
  _map[config->command_enum] = config;
}


/************************************************************************
 ********************** COMMAND PARSING FUNCTIONS ***********************
 ************************************************************************/

void NOP_Parser::operator()(ParserArgs& args)
{
    args.operations.add_op(NOP());
    TokenParser::terminate(args.code_stream, args.engine_interactor.status,
        "NOP expects no args");
}

void LoadConstantParser::operator()(ParserArgs &args)
{
  cpu_word_size const register_idx =
      TokenParser::register_idx(args.code_stream);
  cpu_word_size const value = TokenParser::integer(args.code_stream);
  args.operations.add_op(
      LoadConstantOp(args.brain_interactor.get_register(register_idx), value));
  TokenParser::terminate(args.code_stream, args.engine_interactor.status,
         "Load constant instruction only accepts 2 arguments");
}

void MoveAntParser::operator()(ParserArgs& args)
{
    int dx = 0, dy = 0;

    TokenParser::direction(args.code_stream, dx, dy, args.engine_interactor.status);
    if (args.engine_interactor.status.p_err) return;

    args.operations.add_op(MoveOp(args.engine_interactor, dx, dy));

    TokenParser::terminate(args.code_stream, args.engine_interactor.status, "Move ant operator expects 1 arguments.");
}

void JumpParser::operator()(ParserArgs& args)
{
    std::string word;
    std::string address;
    args.code_stream >> address;
    if( !args.code_stream ) {
        args.engine_interactor.status.error("NEED DIRECTION DEFINED FOR JMP COMMAND");
        return;
    }
    bool is_op_idx = std::find_if(address.begin(), address.end(), [](unsigned char c) {
            return !std::isdigit(c);
    }) == address.end();

    if( is_op_idx ) {
        args.operations.add_op(JmpOp(std::stol(address.c_str()), args.operations));
    } else {
        args.operations.add_op(JmpOp(address, args.operations));
    }

    TokenParser::terminate(args.code_stream, args.engine_interactor.status, "JMP TAKES ONLY ONE ARGUMENT");
}