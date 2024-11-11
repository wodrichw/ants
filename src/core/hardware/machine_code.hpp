#pragma once

#include <vector>

#include "hardware.pb.h"
#include "hardware/label_map.hpp"

using uchar = unsigned char;

struct MachineCode {
    LabelMap labels;
    std::vector<uchar> code;

    MachineCode()=default;
    MachineCode(const ant_proto::MachineCode& msg) : labels(msg.labels()) {
        for( const auto& byte: msg.code() )
            code.emplace_back(byte);
        // for( const auto& instruction: msg.instructions() )
        //     for( int j = 3; j >= 0; --j )
        //         code.emplace_back((instruction >> (j * 8)) & 255);
        // // cleanup possible empty instructions at end of code
        // while( code.size() && static_cast<int>(code[code.size()-1]) ==  0 ) code.pop_back();
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

    ant_proto::MachineCode get_proto() {
        ant_proto::MachineCode msg;
        *msg.mutable_labels() = labels.get_proto();
        std::string str_code;
        str_code.append(code.begin(), code.end());
        *msg.mutable_code() = str_code;
        // for( size_t i = 0; i < code.size(); ++i )
        //     (*msg.mutable_code())[i] = code[i];
        // for(size_t i = 0; i < code.size();) {
        //     uint instr = 0;
        //     for(int j = 0; j < 4; ++j, ++i) {
        //         instr <<= 8;
        //         instr |= (i < code.size() ? code[i] : 0);
        //     }
        //     msg.add_instructions(instr);
        // }

        return msg;
    }
};

