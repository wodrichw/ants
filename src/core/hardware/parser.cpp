#include "hardware/parser.hpp"

#include <sstream>

#include "hardware/operations.hpp"
#include "hardware/parse_args.hpp"
#include "spdlog/spdlog.h"

ParserStatus::ParserStatus() : p_err(false), err_msg("") {
    SPDLOG_DEBUG("ParserStatus created");
}
ParserStatus::ParserStatus(bool p_err, std::string err_msg)
    : p_err(p_err), err_msg(err_msg) {
        SPDLOG_DEBUG("ParserStatus created with error message");
    }
void ParserStatus::error(const std::string& err_msg) {
    SPDLOG_ERROR("ParserStatus error: {}", err_msg);
    this->p_err = true;
    this->err_msg = err_msg;
}

Parser::Parser(CommandMap &command_map,
           std::unordered_set<Command> command_set,
           std::vector<std::string> &program_code,
           ParserArgs &args)
    : commands() {

    SPDLOG_DEBUG("Creating command parser");
    assemble_commands(command_map, command_set);
    parse(program_code, args);
    SPDLOG_TRACE("Command parser created");
}

void Parser::assemble_commands(
    CommandMap &command_map,
    std::unordered_set<Command> command_set) {

    SPDLOG_DEBUG("Assembling commands");
    for(auto &command : command_set) {
        std::string const& command_string = command_map[command]->command_string;
        SPDLOG_TRACE("Inserting command: {}", command_string);
        commands.insert({command_string, command_map[command]->assemble});
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
        ParseLineArgs parse_line{word_stream, status, args};
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
