#include "hardware/command_parsers.hpp"

#include "hardware/brain.hpp"
#include "hardware/command_config.hpp"
#include "hardware/op_def.hpp"
#include "hardware/parse_args.hpp"
#include "hardware/program_executor.hpp"
#include "hardware/token_parser.hpp"
#include "spdlog/spdlog.h"

using uchar = unsigned char;
using schar = signed char;

void NOP_Parser::operator()(ParseArgs &args) {
    SPDLOG_TRACE("Parsing NOP command");
    args.code.push_back(CommandEnum::NOP);
    TokenParser::terminate(args.code_stream, args.status,
                           "NOP expects no args");
    SPDLOG_TRACE("NOP command parsed");
}

void NOP_Deparser::operator()(DeparseArgs &args) {
    SPDLOG_TRACE("Parsing NOP command");
    args.lines.push_back("NOP");
    ++args.code_it;
    SPDLOG_TRACE("NOP command parsed");
}

void LoadConstantParser::operator()(ParseArgs &args) {
    SPDLOG_TRACE("Deparsing Load Constant command");
    uchar instruction = CommandEnum::LOAD;
    uchar const register_idx = TokenParser::register_idx(args.code_stream);
    cpu_word_size const value = TokenParser::integer(args.code_stream);

    // Instruction + register
    args.code.push_back((instruction << 3) | (register_idx & 1));

    // Constant
    args.code.push_back(value & 0xFF);
    args.code.push_back((value >> 8) & 0xFF);
    args.code.push_back((value >> 16) & 0xFF);
    args.code.push_back((value >> 24) & 0xFF);

    TokenParser::terminate(
        args.code_stream, args.status,
        "Load constant instruction only accepts 2 arguments");
    SPDLOG_TRACE("Load Constant command parsed");
}

void LoadConstantDeparser::operator()(DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing Load Constant command");

    uchar const register_name = 'A' + ((*(args.code_it++)) & 1);

    uchar const v0 = *(args.code_it++);
    uchar const v1 = *(args.code_it++);
    uchar const v2 = *(args.code_it++);
    uchar const v3 = *(args.code_it++);
    cpu_word_size const value = v0 | (v1 << 8) | (v2 << 16) << (v3 << 24);

    std::stringstream ss;
    ss << "LDI " << register_name << " " << value;
    args.lines.push_back(ss.str());
    SPDLOG_TRACE("Load Constant command deparsed");
}

void CopyParser::operator()(ParseArgs &args) {
    SPDLOG_TRACE("Parsing Copy command");
    uchar const instruction = CommandEnum::COPY;
    uchar const reg_src_idx = TokenParser::register_idx(args.code_stream);
    uchar const reg_dst_idx = TokenParser::register_idx(args.code_stream);

    args.code.push_back((instruction << 3) | ((reg_src_idx << 1) & 1) |
                        (reg_dst_idx & 1));

    TokenParser::terminate(args.code_stream, args.status,
                           "Copy instruction only accepts 2 arguments");
    SPDLOG_TRACE("Copy command parsed");
}

void CopyDeparser::operator()(DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing Copy command");
    uchar const register_names = *(args.code_it++);
    uchar const reg_src = 'A' + ((register_names >> 1) & 1);
    uchar const reg_dst = 'A' + (register_names & 1);
    std::stringstream ss;
    ss << "COPY " << reg_src << " " << reg_dst;
    args.lines.push_back(ss.str());
    SPDLOG_TRACE("Copy command deparsed");
}

void AddParser::operator()(ParseArgs &args) {
    SPDLOG_TRACE("Parsing Add command");
    uchar const instruction = CommandEnum::ADD;
    uchar const reg_src_idx = TokenParser::register_idx(args.code_stream);
    uchar const reg_dst_idx = TokenParser::register_idx(args.code_stream);

    args.code.push_back((instruction << 3) | ((reg_src_idx << 1) & 1) |
                        (reg_dst_idx & 1));

    TokenParser::terminate(args.code_stream, args.status,
                           "Add instruction only accepts 2 arguments");
    SPDLOG_TRACE("Add command parsed");
}

void AddDeparser::operator()(DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing Add command");
    uchar const register_names = *(args.code_it++);
    uchar const reg_src = 'A' + ((register_names >> 1) & 1);
    uchar const reg_dst = 'A' + (register_names & 1);
    std::stringstream ss;
    ss << "ADD " << reg_src << " " << reg_dst;
    args.lines.push_back(ss.str());
    SPDLOG_TRACE("Add command deparsed");
}

void SubParser::operator()(ParseArgs &args) {
    SPDLOG_TRACE("Parsing Sub command");
    uchar const instruction = CommandEnum::SUB;
    uchar const reg_src_idx = TokenParser::register_idx(args.code_stream);
    uchar const reg_dst_idx = TokenParser::register_idx(args.code_stream);

    args.code.push_back((instruction << 3) | ((reg_src_idx << 1) & 1) |
                        (reg_dst_idx & 1));

    TokenParser::terminate(args.code_stream, args.status,
                           "Subtraction instruction only accepts 2 arguments");
    SPDLOG_TRACE("Sub command parsed");
}

void SubDeparser::operator()(DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing Sub command");
    uchar const register_names = *(args.code_it++);
    uchar const reg_src = 'A' + ((register_names >> 1) & 1);
    uchar const reg_dst = 'A' + (register_names & 1);
    std::stringstream ss;
    ss << "SUB " << reg_src << " " << reg_dst;
    args.lines.push_back(ss.str());
    SPDLOG_TRACE("Sub command deparsed");
}

void IncParser::operator()(ParseArgs &args) {
    SPDLOG_TRACE("Parsing Increment command");
    uchar const instruction = CommandEnum::INC;
    uchar const register_idx = TokenParser::register_idx(args.code_stream);

    args.code.push_back((instruction << 3) | (register_idx & 1));

    TokenParser::terminate(args.code_stream, args.status,
                           "Increment instruction only accepts 2 arguments");
    SPDLOG_TRACE("Increment command parsed");
}

void IncDeparser::operator()(DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing Increment command");
    uchar const reg_name = 'A' + ((*(args.code_it++)) & 1);

    std::stringstream ss;
    ss << "INC " << reg_name;
    args.lines.push_back(ss.str());

    SPDLOG_TRACE("Increment command deparsed");
}

void DecParser::operator()(ParseArgs &args) {
    SPDLOG_TRACE("Parsing Decrement command");
    uchar const instruction = CommandEnum::DEC;
    uchar const register_idx = TokenParser::register_idx(args.code_stream);
    args.code.push_back((instruction << 3) | (register_idx & 1));

    TokenParser::terminate(args.code_stream, args.status,
                           "Decrement instruction only accepts 2 arguments");
    SPDLOG_TRACE("Decrement command parsed");
}

void DecDeparser::operator()(DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing Decrement command");
    uchar const reg_name = 'A' + ((*(args.code_it++)) & 1);

    std::stringstream ss;
    ss << "DEC " << reg_name;
    args.lines.push_back(ss.str());

    SPDLOG_TRACE("Decrement command deparsed");
}

void MoveAntParser::operator()(ParseArgs &args) {
    SPDLOG_TRACE("Parsing Move EntityData command");
    uchar const instruction = CommandEnum::MOVE;
    schar dx = 0, dy = 0;

    TokenParser::direction(args.code_stream, dx, dy, args.status);
    if(args.status.p_err) return;

    // (0, 1) (1, 0) (0, -1) (-1, 0) [START]
    // (1, 2) (2, 1) (1,  0) ( 0, 1) [+1, +1]
    // (0, 2) (1, 1) (0,  0) ( 0, 1) [/2, --]
    // (0, 2) (2, 1) (0,  0) ( 0, 1) [x2, --]
    //  2      3      0        1

    args.code.push_back(((instruction << 3) | (((dx + 1) / 2) * 2)) + (dy + 1));

    TokenParser::terminate(args.code_stream, args.status,
                           "Move ant operator expects 1 arguments.");
    SPDLOG_TRACE("Move EntityData command parsed");
}

void MoveAntDeparser::operator()(DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing Move Ant command");
    uchar const movement = (*(args.code_it++)) & 0b11;

    std::stringstream ss;
    ss << "MOVE ";

    switch(movement) {
        case 0:
            ss << "UP";
            break;
        case 1:
            ss << "LEFT";
            break;
        case 2:
            ss << "DOWN";
            break;
        default:
            ss << "RIGHT";
            break;
    }

    std::string const &dir = ss.str();
    args.lines.push_back(dir);
    SPDLOG_TRACE("Move Ant command deparsed: {}", dir);
}

void DigAntParser::operator()(ParseArgs &args) {
    SPDLOG_TRACE("Parsing Dig EntityData command");
    uchar const instruction = CommandEnum::DIG;
    schar dx = 0, dy = 0;

    TokenParser::direction(args.code_stream, dx, dy, args.status);
    if(args.status.p_err) return;

    // (0, 1) (1, 0) (0, -1) (-1, 0) [START]
    // (1, 2) (2, 1) (1,  0) ( 0, 1) [+1, +1]
    // (0, 2) (1, 1) (0,  0) ( 0, 1) [/2, --]
    // (0, 2) (2, 1) (0,  0) ( 0, 1) [x2, --]
    //  2      3      0        1

    args.code.push_back(((instruction << 3) | (((dx + 1) / 2) * 2)) + (dy + 1));

    TokenParser::terminate(args.code_stream, args.status,
                           "Dig ant operator expects 1 arguments.");
    SPDLOG_TRACE("Dig EntityData command parsed");
}

void DigAntDeparser::operator()(DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing Dig Ant command");
    uchar const movement = (*(args.code_it++)) & 0b11;

    std::stringstream ss;
    ss << "DIG ";

    switch(movement) {
        case 0:
            ss << "UP";
            break;
        case 1:
            ss << "LEFT";
            break;
        case 2:
            ss << "DOWN";
            break;
        default:
            ss << "RIGHT";
            break;
    }

    std::string const &dir = ss.str();
    args.lines.push_back(dir);
    SPDLOG_TRACE("Dig Ant command deparsed: {}", dir);
}

void JumpParser::operator()(ParseArgs &args) {
    SPDLOG_TRACE("Parsing Jump command");
    uchar const instruction = CommandEnum::JMP;
    std::string label = TokenParser::get_label(args.code_stream, args.status);
    ushort const label_idx = args.labels.at(label);

    args.code.push_back(instruction << 3);
    args.code.push_back(label_idx & 0xFF);
    args.code.push_back((label_idx >> 8) & 0xFF);

    TokenParser::terminate(args.code_stream, args.status,
                           "JMP TAKES ONLY ONE ARGUMENT");
    SPDLOG_TRACE("Jump command parsed");
}

void JumpDeparser::operator()(DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing Jump command");

    // Skip the instruction byte
    ++args.code_it;

    ushort const label_idx = (*(args.code_it++)) | ((*(args.code_it++)) << 8);
    std::string const &label = args.labels.at(label_idx);

    std::stringstream ss;
    ss << "JMP " << label;
    args.lines.push_back(ss.str());
    SPDLOG_TRACE("Jump command deparsed");
}

void JumpNotZeroParser::operator()(ParseArgs &args) {
    SPDLOG_TRACE("Parsing Jump Not Zero command");
    uchar const instruction = CommandEnum::JNZ;
    std::string label = TokenParser::get_label(args.code_stream, args.status);
    ushort const label_idx = args.labels.at(label);

    args.code.push_back(instruction << 3);
    args.code.push_back(label_idx & 0xFF);
    args.code.push_back((label_idx >> 8) & 0xFF);

    // args.code.push_back(JnzOp(args.program_executor.op_idx, label_idx,
    // args.registers.zero_flag));

    TokenParser::terminate(args.code_stream, args.status,
                           "JNZ TAKES ONLY ONE ARGUMENT");
    SPDLOG_TRACE("Jump Not Zero command parsed");
}

void JumpNotZeroDeparser::operator()(DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing Jump Not Zero command");

    // Skip the instruction byte
    ++args.code_it;

    ushort const label_idx = (*(args.code_it++)) | ((*(args.code_it++)) << 8);
    std::string const &label = args.labels.at(label_idx);

    std::stringstream ss;
    ss << "JNZ " << label;
    args.lines.push_back(ss.str());
    SPDLOG_TRACE("Jump Not Zero command deparsed");
}
