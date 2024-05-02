#include "hardware/parser.hpp"

#include <climits>
#include <cstdlib>
#include <ios>
#include <sstream>

#include "hardware/operations.hpp"
#include "hardware/op_def.hpp"
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

std::string TokenParser::get_label(std::istringstream &ss, ParserStatus &status) {
    std::string word;
    ss >> word;
    if(!ss) {
        status.error("NEED DIRECTION DEFINED FOR JMP COMMAND");
        return "";
    }
    SPDLOG_DEBUG("Parsed address: {}", word);
    return word;
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
           std::vector<std::string> &program_code,
           ParserArgs &args)
    : commands() {

    SPDLOG_DEBUG("Creating command parser");
    assemble_commands(commands_assember, command_set);
    parse(program_code, args);
    SPDLOG_TRACE("Command parser created");
}

void Parser::assemble_commands(
    ParserCommandsAssembler &commands_assember,
    std::unordered_set<Command> command_set) {

    SPDLOG_DEBUG("Assembling commands");
    for(auto &command : command_set) {
        std::string const& command_string = commands_assember[command].command_string;
        SPDLOG_TRACE("Inserting command: {}", command_string);
        commands.insert({command_string, commands_assember[command].assemble});
    }
    SPDLOG_TRACE("Commands assembled");

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
    operations.add_label(label, operations.size());
    SPDLOG_DEBUG("Added label: {}", label);
    return true;
}

void Parser::parse(std::vector<std::string> &program_code, ParserArgs &args) {
    SPDLOG_INFO("Parsing program code");
    std::unordered_map<std::string, size_t> label_map;
    bool empty_program = true;
    for(std::string &line : program_code) {
        SPDLOG_DEBUG("Parsing line: {}", line);
        std::istringstream word_stream(line);
        ParseLine parse_line{word_stream, status, args};
        CommentHandler handle_comment;
        do {
            std::string word;
            word_stream >> word;
            SPDLOG_TRACE("Extracted word: {}", word);

            if(!word_stream) {
                SPDLOG_TRACE("Empty line detected");
                continue;  // no need to read an empty line
            }
            if(handle_comment(word)) {
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
            c->second(parse_line);          // parse command
            if(status.p_err) return;  // return if error parsing command
        } while(word_stream && !handle_comment.has_comment);
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
    std::function<void(ParseLine &args)> assemble)
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

void NOP_Parser::operator()(ParseLine &parse_line) {
    SPDLOG_TRACE("Parsing NOP command");
    ParserArgs &args = parse_line.args;
    args.operations.add_op(NOP());
    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "NOP expects no args");
    SPDLOG_TRACE("NOP command parsed");
}

void LoadConstantParser::operator()(ParseLine &parse_line) {
    SPDLOG_TRACE("Parsing Load Constant command");
    ParserArgs &args = parse_line.args;
    long const register_idx = TokenParser::register_idx(parse_line.code_stream);
    long const value = TokenParser::integer(parse_line.code_stream);

    args.operations.add_op(
        LoadConstantOp(args.registers[register_idx], args.registers.zero_flag, value));
    TokenParser::terminate(
        parse_line.code_stream, parse_line.status,
        "Load constant instruction only accepts 2 arguments");
    SPDLOG_TRACE("Load Constant command parsed");
}

void CopyParser::operator()(ParseLine &parse_line) {
    SPDLOG_TRACE("Parsing Copy command");
    ParserArgs &args = parse_line.args;
    long const reg_src_idx = TokenParser::register_idx(parse_line.code_stream);
    long const reg_dst_idx = TokenParser::register_idx(parse_line.code_stream);
    args.operations.add_op(
        CopyOp(args.registers[reg_src_idx], args.registers[reg_dst_idx],
               args.registers.zero_flag));
    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "Copy instruction only accepts 2 arguments");
    SPDLOG_TRACE("Copy command parsed");
}

void AddParser::operator()(ParseLine &parse_line) {
    SPDLOG_TRACE("Parsing Add command");
    ParserArgs &args = parse_line.args;
    long const reg_src_idx = TokenParser::register_idx(parse_line.code_stream);
    long const reg_dst_idx = TokenParser::register_idx(parse_line.code_stream);
    args.operations.add_op(
        AddOp(args.registers[reg_src_idx], args.registers[reg_dst_idx],
              args.registers.zero_flag));
    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "Add instruction only accepts 2 arguments");
    SPDLOG_TRACE("Add command parsed");
}

void SubParser::operator()(ParseLine &parse_line) {
    SPDLOG_TRACE("Parsing Sub command");
    ParserArgs &args = parse_line.args;
    long const reg_src_idx = TokenParser::register_idx(parse_line.code_stream);
    long const reg_dst_idx = TokenParser::register_idx(parse_line.code_stream);
    args.operations.add_op(
        SubOp(args.registers[reg_src_idx], args.registers[reg_dst_idx],
              args.registers.zero_flag));
    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "Subtraction instruction only accepts 2 arguments");
    SPDLOG_TRACE("Sub command parsed");
}

void IncParser::operator()(ParseLine &parse_line) {
    SPDLOG_TRACE("Parsing Increment command");
    ParserArgs &args = parse_line.args;
    long const register_idx = TokenParser::register_idx(parse_line.code_stream);
    args.operations.add_op(IncOp(args.registers[register_idx], args.registers.zero_flag));
    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "Increment instruction only accepts 2 arguments");
    SPDLOG_TRACE("Increment command parsed");
}

void DecParser::operator()(ParseLine &parse_line) {
    SPDLOG_TRACE("Parsing Decrement command");
    ParserArgs &args = parse_line.args;
    long const register_idx = TokenParser::register_idx(parse_line.code_stream);
    args.operations.add_op(DecOp(args.registers[register_idx], args.registers.zero_flag));
    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "Decrement instruction only accepts 2 arguments");
    SPDLOG_TRACE("Decrement command parsed");
}

void MoveAntParser::operator()(ParseLine &parse_line) {
    SPDLOG_TRACE("Parsing Move MapData command");
    ParserArgs &args = parse_line.args;
    long dx = 0, dy = 0;

    TokenParser::direction(parse_line.code_stream, dx, dy, parse_line.status);
    if(parse_line.status.p_err) return;

    args.operations.add_op(MoveOp(args.map, args.entity, dx, dy));

    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "Move ant operator expects 1 arguments.");
    SPDLOG_TRACE("Move MapData command parsed");
}

void JumpParser::operator()(ParseLine &parse_line) {
    SPDLOG_TRACE("Parsing Jump command");
    ParserArgs &args = parse_line.args;
    std::string label = TokenParser::get_label(parse_line.code_stream, parse_line.status);
    args.operations.add_op(JmpOp(args.operations.op_idx, args.operations.get_label_idx(label)));

    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "JMP TAKES ONLY ONE ARGUMENT");
    SPDLOG_TRACE("Jump command parsed");
}

void JumpNotZeroParser::operator()(ParseLine &parse_line) {
    SPDLOG_TRACE("Parsing Jump Not Zero command");
    ParserArgs &args = parse_line.args;
    std::string label = TokenParser::get_label(parse_line.code_stream, parse_line.status);
    args.operations.add_op(JnzOp(args.operations.op_idx, args.operations.get_label_idx(label), args.registers.zero_flag));

    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "JNZ TAKES ONLY ONE ARGUMENT");
    SPDLOG_TRACE("Jump Not Zero command parsed");
}
