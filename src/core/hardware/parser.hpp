#pragma once

#include <functional>
#include <string>
#include <unordered_set>

#include "app/globals.hpp"
#include "hardware/command_config.hpp"

struct ParserArgs;
class Operations;

struct ParserStatus {
    bool p_err;
    std::string err_msg;
    void error(const std::string& err_msg);
    ParserStatus();
    ParserStatus(bool p_err, std::string err_msg);
};

class Parser {
   public:
    ParserStatus status;

   private:
    void parse(std::vector<std::string> &program_code, ParserArgs &args);
    bool handle_label(Operations &operations, std::string const &word);
    void assemble_commands(CommandMap& command_map,
        std::unordered_set<Command> command_set);

   public:
    std::unordered_map<std::string, std::function<void(ParseLineArgs &parse_line)>>
        commands;
    Parser(CommandMap &command_map,
           std::unordered_set<Command> command_set,
           std::vector<std::string> &program_code,
           ParserArgs &args);
};
