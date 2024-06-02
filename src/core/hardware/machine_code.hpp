#pragma once

#include <vector>

#include "hardware/label_map.hpp"

using uchar = unsigned char;

struct MachineCode {
    LabelMap labels;
    std::vector<uchar> code;

    MachineCode()=default;
    MachineCode(Unpacker& p) : labels(p) {
        ant_proto::Integer length_msg;
        p >> length_msg;
        int code_length = length_msg.value();
        code.reserve(code_length);

        for(int i = 0; i < code_length;) {
            ant_proto::Integer instr_msg;
            uint instr = instr_msg.value();
            for(int j = 3; j >= 0 && i < code_length; --j, ++i) {
                uchar instr_byte = (instr >> (j * 8)) & 255;
                code.push_back(instr_byte);
            }
        }
    }

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

    friend Packer& operator<<(Packer& p, MachineCode const& obj) {
        ant_proto::Integer length_msg;
        int code_length = obj.code.size();
        length_msg.set_value(code_length);
        p << length_msg;

        for(int i = 0; i < code_length;) {
            uint instr = 0;
            // No need to check i against the code length for simplicity
            for(int j = 0; j < 4; ++j, ++i) {
                instr <<= 8;
                instr |= (i < code_length ? obj.code[i] : 0);
            }
            ant_proto::Integer instr_msg;
            instr_msg.set_value(instr);
            p << instr_msg;
        }
        return p;
    }
};
