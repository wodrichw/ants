#pragma once

#include <stddef.h>

#include "app/globals.hpp"
#include "utils/serializer.hpp"
#include "proto/hardware.pb.h"

struct DualRegisters {
    cpu_word_size registers[2] = {0,0};
    bool zero_flag = 0;

    DualRegisters()=default;
    DualRegisters(Unpacker& p) {
        ant_proto::DualRegisters msg;
        p >> msg;

        registers[0] = msg.register0();
        registers[1] = msg.register1();
        zero_flag = msg.zero_flag();
        SPDLOG_TRACE("Unpacking dual registers - registers: [{}, {}] - zero_flag: {}", registers[0], registers[1], zero_flag ? "ON" : "OFF");
    }
    cpu_word_size& operator[](size_t idx) { return registers[idx]; }
    cpu_word_size const& operator[](size_t idx) const { return registers[idx]; }

    friend Packer& operator<<(Packer& p, DualRegisters const& obj) {
        SPDLOG_TRACE("Packing dual registers - registers: [{}, {}] - zero_flag: {}", obj.registers[0], obj.registers[1], obj.zero_flag ? "ON" : "OFF");
        ant_proto::DualRegisters msg;
        msg.set_register0(obj.registers[0]);
        msg.set_register1(obj.registers[1]);
        msg.set_zero_flag(obj.zero_flag);
        return p << msg;
    }
};
