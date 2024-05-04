#include "hardware/command_parsers.hpp"
#include "hardware/parse_args.hpp"
#include "hardware/token_parser.hpp"
#include "hardware/op_def.hpp"
#include "hardware/operations.hpp"
#include "hardware/brain.hpp"

#include "spdlog/spdlog.h"

void NOP_Parser::operator()(ParseLineArgs &parse_line) {
    SPDLOG_TRACE("Parsing NOP command");
    ParserArgs &args = parse_line.args;
    args.operations.add_op(NoOP());
    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "NOP expects no args");
    SPDLOG_TRACE("NOP command parsed");
}

void LoadConstantParser::operator()(ParseLineArgs &parse_line) {
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

void CopyParser::operator()(ParseLineArgs &parse_line) {
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

void AddParser::operator()(ParseLineArgs &parse_line) {
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

void SubParser::operator()(ParseLineArgs &parse_line) {
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

void IncParser::operator()(ParseLineArgs &parse_line) {
    SPDLOG_TRACE("Parsing Increment command");
    ParserArgs &args = parse_line.args;
    long const register_idx = TokenParser::register_idx(parse_line.code_stream);
    args.operations.add_op(IncOp(args.registers[register_idx], args.registers.zero_flag));
    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "Increment instruction only accepts 2 arguments");
    SPDLOG_TRACE("Increment command parsed");
}

void DecParser::operator()(ParseLineArgs &parse_line) {
    SPDLOG_TRACE("Parsing Decrement command");
    ParserArgs &args = parse_line.args;
    long const register_idx = TokenParser::register_idx(parse_line.code_stream);
    args.operations.add_op(DecOp(args.registers[register_idx], args.registers.zero_flag));
    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "Decrement instruction only accepts 2 arguments");
    SPDLOG_TRACE("Decrement command parsed");
}

void MoveAntParser::operator()(ParseLineArgs &parse_line) {
    SPDLOG_TRACE("Parsing Move EntityData command");
    ParserArgs &args = parse_line.args;
    long dx = 0, dy = 0;

    TokenParser::direction(parse_line.code_stream, dx, dy, parse_line.status);
    if(parse_line.status.p_err) return;

    args.operations.add_op(MoveOp(args.map, args.entity, dx, dy));

    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "Move ant operator expects 1 arguments.");
    SPDLOG_TRACE("Move EntityData command parsed");
}

void JumpParser::operator()(ParseLineArgs &parse_line) {
    SPDLOG_TRACE("Parsing Jump command");
    ParserArgs &args = parse_line.args;
    std::string label = TokenParser::get_label(parse_line.code_stream, parse_line.status);
    args.operations.add_op(JmpOp(args.operations.op_idx, args.operations.get_label_idx(label)));

    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "JMP TAKES ONLY ONE ARGUMENT");
    SPDLOG_TRACE("Jump command parsed");
}

void JumpNotZeroParser::operator()(ParseLineArgs &parse_line) {
    SPDLOG_TRACE("Parsing Jump Not Zero command");
    ParserArgs &args = parse_line.args;
    std::string label = TokenParser::get_label(parse_line.code_stream, parse_line.status);
    size_t label_idx = args.operations.get_label_idx(label);
    SPDLOG_TRACE("JNZ jumps to op idx: {}", label_idx);
    args.operations.add_op(JnzOp(args.operations.op_idx, label_idx, args.registers.zero_flag));

    TokenParser::terminate(parse_line.code_stream, parse_line.status,
                           "JNZ TAKES ONLY ONE ARGUMENT");
    SPDLOG_TRACE("Jump Not Zero command parsed");
}
