#include "hardware/token_parser.hpp"

#include <limits>

#include "spdlog/spdlog.h"

using uchar = unsigned char;

cpu_word_size TokenParser::integer(std::istringstream &ss) {
    Status status;
    return TokenParser::integer(ss, status);
}

cpu_word_size TokenParser::integer(std::istringstream &ss, Status &status) {
    std::string word;
    ss >> word;
    if(!ss || word.empty()) {
        status.error("Expecting an integer argument - none given");
        return 0;
    }

    if(word[0] == '-') {
        status.error("Expecting a non-negative integer argument");
        return 0;
    }

    try {
        size_t idx = 0;
        long long value = std::stoll(word, &idx, 10);
        if(idx != word.size()) {
            status.error("Invalid integer argument");
            return 0;
        }
        if(value < 0 ||
           value > static_cast<long long>(
                       std::numeric_limits<cpu_word_size>::max())) {
            status.error("Integer argument out of range");
            return 0;
        }
        return static_cast<cpu_word_size>(value);
    } catch(const std::exception &ex) {
        status.error("Invalid integer argument");
        SPDLOG_ERROR("Invalid integer token: {}", word);
        return 0;
    }
}

uchar TokenParser::letter_idx(std::istringstream &ss) {
    Status status;
    return TokenParser::letter_idx(ss, status);
}

uchar TokenParser::letter_idx(std::istringstream &ss, Status &status) {
    std::string word;
    ss >> word;
    if(!ss || word.empty()) {
        status.error("Expecting a register/scent argument - none given");
        return 0;
    }
    if(word.size() != 1) {
        status.error("Invalid register/scent argument");
        return 0;
    }

    uchar firstChar = static_cast<uchar>(word[0]);
    if(firstChar >= 'A' && firstChar <= 'Z') {
        SPDLOG_TRACE("Parsed register index: {}", firstChar - 'A');
        return static_cast<uchar>(firstChar - 'A');
    }

    SPDLOG_ERROR("Invalid register index: {}", word);
    status.error("Invalid register/scent argument");
    return 0;
}

void TokenParser::direction(std::istringstream &ss, schar &dx, schar &dy,
                            Status &status) {
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

std::string TokenParser::get_label(std::istringstream &ss, Status &status) {
    std::string word;
    ss >> word;
    if(!ss) {
        status.error("Expecting a label argument - none given");
        return "";
    }
    SPDLOG_DEBUG("Parsed address: {}", word);
    return word;
}

schar TokenParser::get_signed_byte(std::istringstream &ss, Status &status) {
    std::string word;
    ss >> word;
    if(!ss || word.empty()) {
        status.error("Expecting a signed integer argument - none given");
        return 0;
    }

    try {
        size_t idx = 0;
        long long value = std::stoll(word, &idx, 10);
        if(idx != word.size()) {
            status.error("Invalid signed integer argument");
            return 0;
        }
        if(value < std::numeric_limits<schar>::min() ||
           value > std::numeric_limits<schar>::max()) {
            status.error("Signed integer argument out of range");
            return 0;
        }
        schar number = static_cast<schar>(value);
        SPDLOG_DEBUG("Parsed number: {}", number);
        return number;
    } catch(const std::exception &ex) {
        status.error("Invalid signed integer argument");
        SPDLOG_ERROR("Invalid signed integer token: {}", word);
        return 0;
    }
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