#include "parser.hpp"

#include <climits>
#include <cstdlib>
#include <ios>
#include <sstream>

#include "ant_interactor.hpp"
#include "operations.hpp"
#include "spdlog/spdlog.h"

cpu_word_size TokenParser::integer(std::istringstream &ss) {
    std::string word;
    ss >> word;
    return std::stoi(word);
}
cpu_word_size TokenParser::register_idx(std::istringstream &ss) {
    char firstChar;
    ss >> firstChar;

    if(firstChar >= 'A' && firstChar <= 'Z') {
        SPDLOG_TRACE("Parsed register index: {}", firstChar - 'A');
        return firstChar - 'A';
    }

    // Handle the case when the character is not a capital letter
    // You can choose to throw an exception, return a default value, or handle
    // it in a different way based on your requirements. For example, returning
    // a specific value like UINT_MAX to indicate an error.
    SPDLOG_ERROR("Invalid register index: {}", firstChar);
    return UINT_MAX;
}
void TokenParser::direction(std::istringstream &ss, long &dx, long &dy,
                            ParserStatus &status) {
    std::string word;
    ss >> word;

    if(word == "UP") {
        dy = -1;
    } else if(word == "LEFT") {
        dx = -1;
    } else if(word == "DOWN") {
        dy = 1;
    } else if(word == "RIGHT") {
        dx = 1;
    } else {
        status.error(
            "Invalid direction keyword - acceptable directions are: UP, "
            "LEFT, DOWN and RIGHT.");
    }
    SPDLOG_DEBUG("Parsed direction from {}: dx: {}, dy: {}", word, dx, dy);
}

void TokenParser::get_label_address(std::istringstream &ss,
                                    std::string &address, bool &is_op_idx,
                                    ParserStatus &status) {
    std::string word;
    ss >> address;
    if(!ss) {
        status.error("NEED DIRECTION DEFINED FOR JMP COMMAND");
        return;
    }
    is_op_idx =
        std::find_if(address.begin(), address.end(), [](unsigned char c) {
            return !std::isdigit(c);
        }) == address.end();
    SPDLOG_DEBUG("Parsed address: {}", address);
}

void TokenParser::terminate(std::istringstream &ss, ParserStatus &status,
                            const std::string &err_msg) {
    std::string word;
    ss >> word;
    if(ss && word[0] != '#') {
        status.error(err_msg);
    }
    SPDLOG_DEBUG("Terminated parsing with message: {}", err_msg);
}

Parser::Parser(ParserCommandsAssembler &commands_assember,
               std::unordered_set<Command> command_set,
               AntInteractor &ant_interactor, Operations &operations,
               std::vector<std::string> &program_code)
    : commands() {
    SPDLOG_DEBUG("Creating command parser");
    for(auto &command : command_set) {
        SPDLOG_TRACE("Inserting command: {}", commands_assember[command].command_string);
        commands.insert({commands_assember[command].command_string,
                         commands_assember[command].assemble});
    }

    parse(ant_interactor, operations, program_code);
    SPDLOG_TRACE("Command parser created");
}

struct CommentHandler {
    bool has_comment = false;
    bool operator()(std::string const &word) {
        if(word[0] != '#') return false;
        return has_comment = true;
    }
};

bool Parser::handle_label(Operations &operations, std::string const &word) {
    SPDLOG_DEBUG("Handling label: {}", word);
    if(word[word.length() - 1] != ':') {
        SPDLOG_TRACE("Word is not a label");
        return false;
    }

    if(word.length() == 1) {
        status.error("DEFINED AN EMPTY LABEL");
        return false;
    }
    std::string label(word.substr(0, word.length() - 1));
    operations.add_label({label, operations.size()});
    SPDLOG_DEBUG("Added label: {}", label);
    return true;
}

void Parser::parse(AntInteractor &ant_interactor, Operations &operations,
                   std::vector<std::string> &program_code) {
    SPDLOG_INFO("Parsing program code");
    std::unordered_map<std::string, size_t> label_map;
    bool empty_program = true;
    for(std::string &line : program_code) {
        SPDLOG_DEBUG("Parsing line: {}", line);
        std::istringstream word_stream(line);
        ParserArgs args{ant_interactor, operations, word_stream, status};
        CommentHandler comment_handler;
        do {
            std::string word;
            word_stream >> word;
            SPDLOG_TRACE("Extracted word: {}", word);

            if(!word_stream) {
                SPDLOG_TRACE("Empty line detected");
                continue;  // no need to read an empty line
            }
            if(comment_handler(word)) {
                SPDLOG_TRACE("Comment detected");
                continue;
            }

            empty_program = false;

            if(handle_label(args.operations, word)) {
                SPDLOG_DEBUG("Label handled: {}", word);
                continue;
            }
            if(status.p_err) {
                SPDLOG_TRACE("Terminate parsing due to error");
                return;
            }

            auto c = commands.find(word);
            if(c == commands.end()) {
                std::stringstream error_stream;
                error_stream << "THE FOLLOWING IS NOT A VALID COMMAND: "
                             << word;
                status.error(error_stream.str());
                return;
            }
            c->second(args);          // parse command
            if(status.p_err) return;  // return if error parsing command
        } while(word_stream && !comment_handler.has_comment);
    }
    if(empty_program) {
        status.error("WILL NOT ADD AN ANT WITHOUT A PROGRAM");
    }
}
const Parser::CommandConfig &ParserCommandsAssembler::operator[](
    Parser::Command command) {
    return *_map[command];
}
Parser::CommandConfig::CommandConfig(
    const std::string &command_string, Command command_enum,
    std::function<void(ParserArgs &args)> assemble)
    : command_string(command_string),
      command_enum(command_enum),
      assemble(assemble) {
        SPDLOG_TRACE("Creating command config for command: {}", command_string);
      }

ParserCommandsAssembler::ParserCommandsAssembler() : _map() {
    SPDLOG_TRACE("Creating command assembler");
    
    // Empty command
    insert(
        new Parser::CommandConfig("NOP", Parser::Command::NOP, NOP_Parser()));

    // Load constant command to register
    insert(new Parser::CommandConfig("LDI", Parser::Command::LOAD,
                                     LoadConstantParser()));

    // Copy register to register
    insert(
        new Parser::CommandConfig("CPY", Parser::Command::COPY, CopyParser()));

    // Add second register to the first
    insert(new Parser::CommandConfig("ADD", Parser::Command::ADD, AddParser()));

    // Subtract second register from the first
    insert(new Parser::CommandConfig("SUB", Parser::Command::SUB, SubParser()));

    // Increment register
    insert(new Parser::CommandConfig("INC", Parser::Command::INC, IncParser()));

    // Decrement register
    insert(new Parser::CommandConfig("DEC", Parser::Command::DEC, DecParser()));

    // MOVE command
    insert(new Parser::CommandConfig("MOVE", Parser::Command::MOVE,
                                     MoveAntParser()));

    // JMP command
    insert(
        new Parser::CommandConfig("JMP", Parser::Command::JMP, JumpParser()));

    // JNZ command
    insert(new Parser::CommandConfig("JNZ", Parser::Command::JNZ,
                                     JumpNotZeroParser()));
}

void ParserCommandsAssembler::insert(Parser::CommandConfig *config) {
    // SPDLOG_TRACE("Inserting command config for command: {}", config->command_string);
    _map[config->command_enum] = config;
}

/************************************************************************
 ********************** COMMAND PARSING FUNCTIONS ***********************
 ************************************************************************/

void NOP_Parser::operator()(ParserArgs &args) {
    SPDLOG_TRACE("Parsing NOP command");
    args.operations.add_op(NOP());
    TokenParser::terminate(args.code_stream, args.status,
                           "NOP expects no args");
    SPDLOG_TRACE("NOP command parsed");
}

void LoadConstantParser::operator()(ParserArgs &args) {
    SPDLOG_TRACE("Parsing Load Constant command");
    long const register_idx = TokenParser::register_idx(args.code_stream);
    long const value = TokenParser::integer(args.code_stream);

    args.operations.add_op(
        LoadConstantOp(args.ant_interactor, register_idx, value));
    TokenParser::terminate(
        args.code_stream, args.status,
        "Load constant instruction only accepts 2 arguments");
    SPDLOG_TRACE("Load Constant command parsed");
}

void CopyParser::operator()(ParserArgs &args) {
    SPDLOG_TRACE("Parsing Copy command");
    long const reg_src_idx = TokenParser::register_idx(args.code_stream);
    long const reg_dst_idx = TokenParser::register_idx(args.code_stream);
    args.operations.add_op(
        CopyOp(args.ant_interactor, reg_src_idx, reg_dst_idx));
    TokenParser::terminate(args.code_stream, args.status,
                           "Copy instruction only accepts 2 arguments");
    SPDLOG_TRACE("Copy command parsed");
}

void AddParser::operator()(ParserArgs &args) {
    SPDLOG_TRACE("Parsing Add command");
    long const reg_src_idx = TokenParser::register_idx(args.code_stream);
    long const reg_dst_idx = TokenParser::register_idx(args.code_stream);
    args.operations.add_op(
        AddOp(args.ant_interactor, reg_src_idx, reg_dst_idx));
    TokenParser::terminate(args.code_stream, args.status,
                           "Add instruction only accepts 2 arguments");
    SPDLOG_TRACE("Add command parsed");
}

void SubParser::operator()(ParserArgs &args) {
    SPDLOG_TRACE("Parsing Sub command");
    long const reg_src_idx = TokenParser::register_idx(args.code_stream);
    long const reg_dst_idx = TokenParser::register_idx(args.code_stream);
    args.operations.add_op(
        SubOp(args.ant_interactor, reg_src_idx, reg_dst_idx));
    TokenParser::terminate(args.code_stream, args.status,
                           "Subtraction instruction only accepts 2 arguments");
    SPDLOG_TRACE("Sub command parsed");
}

void IncParser::operator()(ParserArgs &args) {
    SPDLOG_TRACE("Parsing Increment command");
    long const register_idx = TokenParser::register_idx(args.code_stream);
    args.operations.add_op(IncOp(args.ant_interactor, register_idx));
    TokenParser::terminate(args.code_stream, args.status,
                           "Increment instruction only accepts 2 arguments");
    SPDLOG_TRACE("Increment command parsed");
}

void DecParser::operator()(ParserArgs &args) {
    SPDLOG_TRACE("Parsing Decrement command");
    long const register_idx = TokenParser::register_idx(args.code_stream);
    args.operations.add_op(DecOp(args.ant_interactor, register_idx));
    TokenParser::terminate(args.code_stream, args.status,
                           "Decrement instruction only accepts 2 arguments");
    SPDLOG_TRACE("Decrement command parsed");
}

void MoveAntParser::operator()(ParserArgs &args) {
    SPDLOG_TRACE("Parsing Move Ant command");
    long dx = 0, dy = 0;

    TokenParser::direction(args.code_stream, dx, dy, args.status);
    if(args.status.p_err) return;

    args.operations.add_op(MoveOp(args.ant_interactor, dx, dy));

    TokenParser::terminate(args.code_stream, args.status,
                           "Move ant operator expects 1 arguments.");
    SPDLOG_TRACE("Move Ant command parsed");
}

void JumpParser::operator()(ParserArgs &args) {
    SPDLOG_TRACE("Parsing Jump command");
    std::string address;
    bool is_op_idx;
    TokenParser::get_label_address(args.code_stream, address, is_op_idx,
                                   args.status);

    if(is_op_idx) {
        args.operations.add_op(
            JmpOp(std::stol(address.c_str()), args.operations));
    } else {
        args.operations.add_op(JmpOp(address, args.operations));
    }

    TokenParser::terminate(args.code_stream, args.status,
                           "JMP TAKES ONLY ONE ARGUMENT");
    SPDLOG_TRACE("Jump command parsed");
}

void JumpNotZeroParser::operator()(ParserArgs &args) {
    SPDLOG_TRACE("Parsing Jump Not Zero command");
    std::string address;
    bool is_op_idx;
    TokenParser::get_label_address(args.code_stream, address, is_op_idx,
                                   args.status);

    if(is_op_idx) {
        args.operations.add_op(JnzOp(
            args.ant_interactor, std::stol(address.c_str()), args.operations));
    } else {
        args.operations.add_op(
            JnzOp(args.ant_interactor, address, args.operations));
    }

    TokenParser::terminate(args.code_stream, args.status,
                           "JNZ TAKES ONLY ONE ARGUMENT");
    SPDLOG_TRACE("Jump Not Zero command parsed");
}
