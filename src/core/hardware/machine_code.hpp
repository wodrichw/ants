#pragma once

#include <vector>

#include "hardware/label_map.hpp"

using uchar = unsigned char;

struct MachineCode {
    LabelMap labels;
    std::vector<uchar> code;

    void clear() {
        labels.clear();
        code.clear();
    }

    bool is_empty() const {
        return code.size() == 0 && labels.size() == 0;
    }

    size_t size() const {
        return code.size();
    }
};
