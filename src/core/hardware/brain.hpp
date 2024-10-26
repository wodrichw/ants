#pragma once

#include <stddef.h>

#include "app/globals.hpp"
#include "utils/serializer.hpp"
#include "proto/hardware.pb.h"

using uchar = unsigned char;
using ushort = unsigned short;

struct DualRegisters {
    cpu_word_size registers[2] = {0,0};
    cpu_word_size ram[64] = {};

    ushort instr_ptr_register = 0;
    ushort base_ptr_register = 0;
    ushort stack_ptr_register = 0;

    bool zero_flag = 0;
    bool instr_failed_flag = 0;

    // Heading flags
    // Direction Flags
    // 0 0 RIGHT
    // 0 1 UP
    // 1 0 LEFT
    // 1 1 DOWN
    bool dir_flag1 = 0, dir_flag2 = 0;

    // sync flags
    bool is_move_flag = 0, is_dig_flag = 0;

    // 4 bits - down(most-sig), left, up, right(lst-sig) (absolute - not relative to heading)
    // all directions are stored so turns can be updated without checking the map
    // which would generate a sync event and halt the async steps
    uchar is_space_empty_flags = 0b1111;
    
    ushort wait_move_tick_count = 12; // 60 FPS / 5 moves per sec = 12
    ushort wait_dig_tick_count = 4; // 60 FPS / 15 digs per sec = 4

    DualRegisters()=default;
    DualRegisters(Unpacker& p) {
        ant_proto::DualRegisters msg;
        p >> msg;

        registers[0] = msg.register0();
        registers[1] = msg.register1();
        instr_ptr_register = msg.instr_ptr_register();
        base_ptr_register = msg.base_ptr_register();
        stack_ptr_register = msg.stack_ptr_register();
        zero_flag = msg.zero_flag();
        dir_flag1 = msg.dir_flag1();
        dir_flag2 = msg.dir_flag2();
        instr_failed_flag = msg.instr_failed_flag();
        is_move_flag = msg.is_move_flag();
        is_dig_flag = msg.is_dig_flag();
        // is_front_space_empty should not be unpacked - will be determined automatically.
        SPDLOG_TRACE("Unpacking dual registers - registers: [{}, {}] - zero_flag: {}", registers[0], registers[1], zero_flag ? "ON" : "OFF");
    }
    cpu_word_size& operator[](size_t idx) { return registers[idx]; }
    cpu_word_size const& operator[](size_t idx) const { return registers[idx]; }

    friend Packer& operator<<(Packer& p, DualRegisters const& obj) {
        SPDLOG_TRACE("Packing dual registers - registers: [{}, {}] - zero_flag: {}", obj.registers[0], obj.registers[1], obj.zero_flag ? "ON" : "OFF");
        ant_proto::DualRegisters msg;
        msg.set_register0(obj.registers[0]);
        msg.set_register1(obj.registers[1]);
        msg.set_instr_ptr_register(obj.instr_ptr_register);
        msg.set_base_ptr_register(obj.base_ptr_register);
        msg.set_stack_ptr_register(obj.stack_ptr_register);
        msg.set_zero_flag(obj.zero_flag);
        msg.set_dir_flag1(obj.dir_flag1);
        msg.set_dir_flag2(obj.dir_flag2);
        msg.set_instr_failed_flag(obj.instr_failed_flag);
        msg.set_is_move_flag(obj.is_move_flag);
        msg.set_is_dig_flag(obj.is_dig_flag);
        // is_front_space_empty should not be packed - will be regenerated automatically.
        return p << msg;
    }
};
