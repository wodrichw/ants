#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "hardware/label_map.hpp"
#include "utils/status.hpp"

using uchar = unsigned char;

struct ParseArgs {
    ParseArgs(std::string const& line, std::vector<uchar>& code,
              LabelMap& labels, Status& status)
        : code_stream(line), code(code), labels(labels), status(status) {}

    std::istringstream code_stream;
    std::vector<uchar>& code;
    LabelMap const& labels;
    Status& status;
};

struct DeparseArgs {
    DeparseArgs(std::vector<uchar>::const_iterator code_it,
                LabelMap const& labels, std::vector<std::string>& lines,
                Status& status)
        : code_it(code_it), labels(labels), lines(lines), status(status) {}

    std::vector<uchar>::const_iterator code_it;
    LabelMap const& labels;
    std::vector<std::string>& lines;
    Status& status;
};
