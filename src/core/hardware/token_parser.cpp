#include "hardware/token_parser.hpp"
#include "hardware/parser.hpp"
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