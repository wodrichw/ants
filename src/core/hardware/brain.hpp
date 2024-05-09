#pragma once

#include <stddef.h>

#include "app/globals.hpp"
#include "utils/serializer.hpp"
#include "proto/hardware.pb.h"

struct DualRegisters {
    cpu_word_size registers[2];
    bool zero_flag;

    DualRegisters()=default;
    DualRegisters(Unpacker& p) {
        ant_proto::DualRegisters msg;
        p >> msg;

        registers[0] = msg.register0();
        registers[1] = msg.register1();
        zero_flag = msg.zero_flag();
    }
    cpu_word_size& operator[](size_t idx) { return registers[idx]; }
    cpu_word_size const& operator[](size_t idx) const { return registers[idx]; }

    friend Packer& operator<<(Packer& p, DualRegisters const& obj) {
        ant_proto::DualRegisters msg;
        msg.set_register0(obj.registers[0]);
        msg.set_register1(obj.registers[1]);
        msg.set_zero_flag(obj.zero_flag);
        return p << msg;
    }
};
