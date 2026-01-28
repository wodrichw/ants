#include "hardware/token_parser.hpp"

#include "spdlog/spdlog.h"

using uchar = unsigned char;

cpu_word_size TokenParser::integer(std::istringstream &ss) {
    std::string word;
    ss >> word;
    return std::stoi(word);
}
cpu_word_size TokenParser::integer(std::istringstream &ss, Status &status) {
    std::string word;
    ss >> word;
    if(!ss) {
        status.error("Expecting an integer argument - none given");
        return 0;
    }

    try {
        return std::stoi(word);
    } catch(const std::exception &) {
        status.error("Invalid integer argument");
        return 0;
    }
}
uchar TokenParser::letter_idx(std::istringstream &ss) {
    uchar firstChar;
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
    return 0b111;
}

uchar TokenParser::letter_idx(std::istringstream &ss, Status &status) {
    uchar firstChar = 0;
    ss >> firstChar;

    if(!ss) {
        status.error("Expecting a register argument - none given");
        return 0;
    }

    if(firstChar >= 'A' && firstChar <= 'Z') {
        SPDLOG_TRACE("Parsed register index: {}", firstChar - 'A');
        return firstChar - 'A';
    }

    SPDLOG_ERROR("Invalid register index: {}", firstChar);
    status.error("Invalid register argument - expected A-Z");
    return 0;
}

void TokenParser::direction(std::istringstream &ss, schar &dx, schar &dy,
                            Status &status) {
    std::string word;
    ss >> word;

    if(!ss) {
        status.error(
            "Expecting a direction argument - acceptable directions are: UP, "
            "LEFT, DOWN and RIGHT.");
        return;
    }

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

std::string TokenParser::get_label(std::istringstream &ss, Status &status) {
    std::string word;
    ss >> word;
    if(!ss) {
        status.error("NEED DIRECTION DEFINED FOR JMP COMMAND");
        return "";
    }
    SPDLOG_DEBUG("Parsed address: {}", word);
    return word;
}

schar TokenParser::get_signed_byte(std::istringstream &ss, Status &status) {
    std::string word;
    ss >> word;
    if(!ss) {
        status.error("Expecting a signed integer argument - none given");
        return 0;
    }

    schar number = std::stoi(word);
    SPDLOG_DEBUG("Parsed number: {}", number);
    return number;
}

void TokenParser::terminate(std::istringstream &ss, Status &status,
                            std::string const &cmd_name,
                            std::string const &err_msg) {
    std::string word;
    ss >> word;
    if(ss) {
        status.error(err_msg);
        SPDLOG_ERROR("Additional characters remained: {} - {}", cmd_name,
                     err_msg);
    }
    (void)cmd_name;
}