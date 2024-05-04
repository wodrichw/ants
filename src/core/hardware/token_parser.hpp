#pragma once

#include <sstream>
#include <string>

#include "app/globals.hpp"

struct ParserStatus;

// Parser helpers
namespace TokenParser {
    // Parse string to integer
    cpu_word_size integer(std::istringstream &ss);

    // Parse register names to an index
    // Register names are single characters A-Z
    // A -> 0
    // B -> 1
    // ...
    cpu_word_size register_idx(std::istringstream &ss);

    // Parse direction keyword to a dx and dy pair
    void direction(std::istringstream &ss, long &dx, long &dy,
                   ParserStatus& status);

    std::string get_label(std::istringstream &ss, ParserStatus& status);

    // Check that no more arguments exist to be parsed.
    void terminate(std::istringstream &ss, ParserStatus& status,
                   const std::string &err_msg);
}  // namespace TokenParser