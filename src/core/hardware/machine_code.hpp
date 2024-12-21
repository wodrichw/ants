#pragma once

#include <vector>

#include "hardware.pb.h"
#include "hardware/label_map.hpp"

using uchar = unsigned char;

struct MachineCode {
    LabelMap labels;
    std::vector<uchar> code;

    MachineCode() = default;
    MachineCode(const ant_proto::MachineCode& msg);

    void clear();
    bool is_empty() const;
    size_t size() const;
    ant_proto::MachineCode get_proto();
};
