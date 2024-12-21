#include "hardware/parser.hpp"

#include <sstream>

#include "hardware/parse_args.hpp"
#include "hardware/machine_code.hpp"
#include "utils/string.hpp"

#include "spdlog/spdlog.h"

using ushort = unsigned short;

Parser::Parser(CommandMap const& command_map)
    : command_map(command_map) {
    SPDLOG_TRACE("Command parser created");
}

void Parser::parse(std::vector<std::string> const& program_code, MachineCode &machine_code, Status& status) {
    SPDLOG_INFO("Parsing program code");

    std::vector<std::string> active_code;
    preprocess(program_code, machine_code.labels, active_code, status);
    if(status.p_err) {
        SPDLOG_TRACE("Terminate parsing due to error");
        return;
    }
    if (active_code.size() == 0) {
        return;
    }

    SPDLOG_INFO("Parsing {} lines of code text", active_code.size());
    for(std::string const& line : active_code) {
        SPDLOG_DEBUG("Parsing line: {}", line);
        ParseArgs args(line, machine_code.code, machine_code.labels, status);

        std::string cmd_str;
        args.code_stream >> cmd_str;
        SPDLOG_TRACE("Extracted word: {}", cmd_str);

        auto it = command_map.find(cmd_str);
        if(it == command_map.str_end()) {
            std::stringstream err;
            err << "THE FOLLOWING IS NOT A VALID COMMAND: " << cmd_str;
            status.error(err.str());
            return;
        }

        CommandConfig const& command = *it->second;
        command.parse(command, args);          // parse command
        if(status.p_err) return;  // return if error parsing command

        SPDLOG_TRACE("Current parsed code size: {} bytes", machine_code.code.size());
    }
    SPDLOG_INFO("Successfully parsed: {} / {} lines -> {} bytes", active_code.size(), program_code.size(), machine_code.code.size());
}

void Parser::deparse(MachineCode const& machine_code, std::vector<std::string>& program_code, Status& status) {
    SPDLOG_INFO("Starting deparsing instructions <- {} bytes", machine_code.code.size());
    DeparseArgs args(machine_code.code.begin(), machine_code.labels, program_code, status);
    while(args.code_it != machine_code.code.end()) {
        uchar instruction_code = *args.code_it >> 3;
        SPDLOG_TRACE("Instruction code: {}", instruction_code);
        CommandEnum instruction = static_cast<CommandEnum>(instruction_code);
        CommandConfig const& command = command_map.at(instruction);
        SPDLOG_DEBUG("Instruction name: {}", command.command_string);
        command.deparse(command, args);

        if (status.p_err) {
            SPDLOG_ERROR("Failed to deparse instruction: {}", command.command_string);
            return;
        }
        SPDLOG_TRACE("Successfully deparsed instruction");
    }

    SPDLOG_DEBUG("Getting addresses to insert instruction labels");
    std::vector<ushort> addresses;
    machine_code.labels.get_addresses(addresses);

    SPDLOG_TRACE("Sorting the label addresses");
    std::sort(addresses.rbegin(), addresses.rend());

    SPDLOG_TRACE("Inserting the labels into the instruction vector");
    for (ushort address: addresses) {
        std::string label = machine_code.labels.at(address) + ":";
        program_code.insert(program_code.begin() + address, label);
    }

    SPDLOG_INFO("Successfully deparsed: {} bytes -> {} lines", machine_code.code.size(), program_code.size());
}

bool Parser::handle_label(LabelMap &labels, std::string const &word, ushort address, Status& status) {
    SPDLOG_TRACE("Checking if label: {}", word);
    if(word[word.length() - 1] != ':') {
        SPDLOG_TRACE("Word is not a label");
        return false;
    }

    if(word.length() == 1) {
        status.error("DEFINED AN EMPTY LABEL");
        return false;
    }
    std::string label(word.substr(0, word.length() - 1));
    labels.insert(address, label);
    SPDLOG_DEBUG("Added label: {}", label);
    return true;
}

void Parser::preprocess(std::vector<std::string> const& program_code, LabelMap& labels, std::vector<std::string>& active_code, Status& status) {
    SPDLOG_DEBUG("Preprocessing string program");
    for(std::string const& raw_line : program_code) {
        std::string line = trim_copy(raw_line);
        if (line.length() == 0 || line[0] == '#') continue;
        if (handle_label(labels, line, active_code.size(), status)) continue;

        active_code.push_back(line);
    }
} 
