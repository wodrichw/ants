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

void NoArgCommandParser::operator()(CommandConfig const& config, ParseArgs& args) {
    SPDLOG_TRACE("Parsing {} command - no args", config.command_string);
    args.code.push_back(config.command_enum << 3);
    TokenParser::terminate(args.code_stream, args.status, config.command_string,
                           "expecting no args");
    SPDLOG_TRACE("{} command parsed", config.command_string);
}

void NoArgCommandDeparser::operator()(CommandConfig const& config, DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing {} command - no args", config.command_string);
    args.lines.push_back(config.command_string);
    ++args.code_it;
    SPDLOG_TRACE("{} command parsed", config.command_string);
}

void LoadConstantParser::operator()(CommandConfig const& config, ParseArgs &args) {
    SPDLOG_TRACE("Parsing {} command", config.command_string);
    uchar instruction = CommandEnum::LOAD;
    uchar const register_idx = TokenParser::letter_idx(args.code_stream);
    cpu_word_size const value = TokenParser::integer(args.code_stream);

    // Instruction + register
    args.code.push_back((instruction << 3) | (register_idx & 1));

    // Constant
    args.code.push_back(value & 0xFF);
    args.code.push_back((value >> 8) & 0xFF);
    args.code.push_back((value >> 16) & 0xFF);
    args.code.push_back((value >> 24) & 0xFF);

    TokenParser::terminate(
        args.code_stream, args.status, config.command_string,
        "expecting 2 args");
    SPDLOG_TRACE("{} command parsed", config.command_string);
}

void LoadConstantDeparser::operator()(CommandConfig const& config, DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing %s command", config.command_string);

    uchar const register_name = 'A' + ((*args.code_it) & 1);

    uchar const v0 = *(++args.code_it);
    uchar const v1 = *(++args.code_it);
    uchar const v2 = *(++args.code_it);
    uchar const v3 = *(++args.code_it);
    cpu_word_size const value = v0 | (v1 << 8) | (v2 << 16) << (v3 << 24);

    std::stringstream ss;
    ss << config.command_string << " " << register_name << " " << value;
    args.lines.push_back(ss.str());
    ++args.code_it;
    SPDLOG_TRACE("{} command deparsed", config.command_string);
}

void TwoRegisterCommandParser::operator()(CommandConfig const& config, ParseArgs &args) {
    SPDLOG_TRACE("Parsing {} command", config.command_string);
    uchar const instruction = config.command_enum;
    uchar const reg_src_idx = TokenParser::letter_idx(args.code_stream);
    uchar const reg_dst_idx = TokenParser::letter_idx(args.code_stream);

    args.code.push_back((instruction << 3) | ((reg_src_idx << 1) & 1) |
                        (reg_dst_idx & 1));

    TokenParser::terminate(args.code_stream, args.status, config.command_string,
                           "expecting 2 arguments");
    SPDLOG_TRACE("{} command parsed", config.command_string);
}

void TwoLetterCommandDeparser::operator()(CommandConfig const& config, DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing {} command", config.command_string);
    uchar const register_names = *args.code_it;
    uchar const reg_src = 'A' + ((register_names >> 1) & 1);
    uchar const reg_dst = 'A' + (register_names & 1);

    std::stringstream ss;
    ss << config.command_string << " " << reg_src << " " << reg_dst;
    args.lines.push_back(ss.str());

    ++args.code_it;
    SPDLOG_TRACE("{} command deparsed", config.command_string);
}

void OneRegisterCommandParser::operator()(CommandConfig const& config, ParseArgs &args) {
    SPDLOG_TRACE("Parsing {} command", config.command_string);
    uchar const instruction = config.command_enum;
    uchar const register_idx = TokenParser::letter_idx(args.code_stream);

    args.code.push_back((instruction << 3) | (register_idx & 1));

    TokenParser::terminate(args.code_stream, args.status, config.command_string,
                           "expecting 2 arguments");
    SPDLOG_TRACE("{} command parsed", config.command_string);
}

void OneLetterCommandDeparser::operator()(CommandConfig const& config, DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing {} command", config.command_string);
    uchar const reg_name = 'A' + ((*args.code_it) & 1);

    std::stringstream ss;
    ss << config.command_string << " " << reg_name;
    args.lines.push_back(ss.str());

    ++args.code_it;
    SPDLOG_TRACE("{} command deparsed", config.command_string);
}

void JumpParser::operator()(CommandConfig const& config, ParseArgs &args) {
    SPDLOG_TRACE("Parsing {} command", config.command_string);
    uchar const instruction = config.command_enum;
    std::string label = TokenParser::get_label(args.code_stream, args.status);
    ushort const label_idx = args.labels.at(label);

    args.code.push_back(instruction << 3);
    args.code.push_back(label_idx & 0xFF);
    args.code.push_back((label_idx >> 8) & 0xFF);

    TokenParser::terminate(args.code_stream, args.status, config.command_string,
                           "expecting 1 argument");
    SPDLOG_TRACE("{} command parsed", config.command_string);
}

void JumpDeparser::operator()(CommandConfig const& config, DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing {} command", config.command_string);

    ushort const label_idx = (*(++args.code_it)) | ((*(++args.code_it)) << 8);
    std::string const &label = args.labels.at(label_idx);

    std::stringstream ss;
    ss << config.command_string << " " << label;
    args.lines.push_back(ss.str());

    ++args.code_it;
    SPDLOG_TRACE("{} command deparsed", config.command_string);
}

void OneScentCommandParser::operator()(CommandConfig const& config, ParseArgs &args) {
    SPDLOG_TRACE("Parsing {} command", config.command_string);
    uchar const instruction = config.command_enum;
    uchar const scent_idx = TokenParser::letter_idx(args.code_stream);

    args.code.push_back((instruction << 3) | (scent_idx & 0b111));

    TokenParser::terminate(args.code_stream, args.status, config.command_string,
                           "expecting 2 arguments");
    SPDLOG_TRACE("{} command parsed", config.command_string);
}

void SetScentPriorityParser::operator()(CommandConfig const& config, ParseArgs &args) {
    SPDLOG_TRACE("Parsing {} command", config.command_string);
    uchar const instruction = config.command_enum;
    uchar const scent_idx = TokenParser::letter_idx(args.code_stream);

    args.code.push_back((instruction << 3) | (scent_idx & 0b111));

    schar priority = TokenParser::get_signed_byte(args.code_stream, args.status);
    args.code.push_back(static_cast<uchar>(priority));

    TokenParser::terminate(args.code_stream, args.status, config.command_string,
                           "expecting 3 arguments");
    SPDLOG_TRACE("{} command parsed", config.command_string);
}

void SetScentPriorityDeparser::operator()(CommandConfig const& config, DeparseArgs &args) {
    SPDLOG_TRACE("Deparsing {} command", config.command_string);
    uchar const reg_name = 'A' + ((*args.code_it) & 1);
    schar const priority = static_cast<schar>(*(++args.code_it));
    std::string sign = priority < 0 ? "-" : "";

    std::stringstream ss;
    ss << config.command_string << " " << reg_name << " " << sign << std::abs(priority);
    args.lines.push_back(ss.str());

    ++args.code_it;
    SPDLOG_TRACE("{} command deparsed", config.command_string);
}