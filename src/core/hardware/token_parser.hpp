#pragma once

#include <sstream>
#include <string>

#include "app/globals.hpp"
#include "utils/status.hpp"

using uchar = unsigned char;
using schar = signed char;

struct Status;

// Parser helpers
namespace TokenParser {
    // Parse string to integer
    cpu_word_size integer(std::istringstream& ss);

    // Parse register names to an index
    // Register names are single characters A-Z
    // A -> 0
    // B -> 1
    // ...
    uchar letter_idx(std::istringstream& ss);

    // Parse direction keyword to a dx and dy pair
    void direction(std::istringstream& ss, schar& dx, schar& dy,
                   Status& status);

    schar get_signed_byte(std::istringstream& ss, Status& status);

    std::string get_label(std::istringstream& ss, Status& status);

    // Check that no more arguments exist to be parsed.
    void terminate(std::istringstream& ss, Status& status,
                   std::string const& cmd_name, std::string const& err_msg);
}  // namespace TokenParser