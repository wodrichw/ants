#pragma once

#include <stddef.h>

#include "app/globals.hpp"
#include "utils/serializer.hpp"
#include "proto/hardware.pb.h"
#include "entity/scents.hpp"

using uchar = unsigned char;
using ushort = unsigned short;

struct DualRegisters {
    cpu_word_size registers[2] = {0,0};
    cpu_word_size ram[64] = {};
    ulong chunk_scents_list[4] = {}; // scents of chunks: right, up, left, down
    ulong delta_scents = 0; // delta scents of current chunk
    ScentBehaviors scent_behaviors;

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

    DualRegisters() : scent_behaviors(is_space_empty_flags) {}
    DualRegisters(const ant_proto::DualRegisters& msg) :
        scent_behaviors(is_space_empty_flags),
        instr_ptr_register(msg.instr_ptr_register()),
        base_ptr_register(msg.base_ptr_register()),
        stack_ptr_register(msg.stack_ptr_register()),
        zero_flag(msg.zero_flag()),
        instr_failed_flag(msg.instr_failed_flag()),
        dir_flag1(msg.dir_flag1()),
        dir_flag2(msg.dir_flag2()),
        is_move_flag(msg.is_move_flag()),
        is_dig_flag(msg.is_dig_flag())
    {
        registers[0] = msg.register0();
        registers[1] = msg.register1();
        SPDLOG_TRACE("Unpacking dual registers - registers: [{}, {}] - zero_flag: {}", registers[0], registers[1], zero_flag ? "ON" : "OFF");
    }

    cpu_word_size& operator[](size_t idx) { return registers[idx]; }
    cpu_word_size const& operator[](size_t idx) const { return registers[idx]; }

    ant_proto::DualRegisters get_proto() {
        ant_proto::DualRegisters msg;
        msg.set_register0(registers[0]);
        msg.set_register1(registers[1]);
        msg.set_instr_ptr_register(instr_ptr_register);
        msg.set_base_ptr_register(base_ptr_register);
        msg.set_stack_ptr_register(stack_ptr_register);
        msg.set_zero_flag(zero_flag);
        msg.set_dir_flag1(dir_flag1);
        msg.set_dir_flag2(dir_flag2);
        msg.set_instr_failed_flag(instr_failed_flag);
        msg.set_is_move_flag(is_move_flag);
        msg.set_is_dig_flag(is_dig_flag);

        return msg;
    }
};
