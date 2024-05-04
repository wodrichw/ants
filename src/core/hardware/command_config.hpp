#pragma once

#include <string>
#include <functional>

struct ParseLineArgs;

enum Command { ADD, COPY, DEC, INC, JNZ, JMP, LOAD, MOVE, NOP, SUB };
struct CommandConfig {
    std::string command_string;
    Command command_enum;
    std::function<void(ParseLineArgs &args)> assemble;
    CommandConfig(const std::string &command_string, Command command_enum,
                    std::function<void(ParseLineArgs &args)> assemble);
};

using CommandMap = std::unordered_map<Command, CommandConfig*>;
void get_command_map(CommandMap& map);
