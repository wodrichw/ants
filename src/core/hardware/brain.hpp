#pragma once

#include <stddef.h>

#include "app/globals.hpp"
#include "entity/scents.hpp"
#include "proto/hardware.pb.h"
#include "utils/serializer.hpp"
#include "utils/types.hpp"


struct DualRegisters {
    cpu_word_size registers[2] = {0, 0};
    cpu_word_size ram[64] = {};
    ulong chunk_scents_list[4] = {};  // scents of chunks: right, up, left, down
    ulong delta_scents = 0;           // delta scents of current chunk
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

    // 4 bits - down(most-sig), left, up, right(lst-sig) (absolute - not
    // relative to heading) all directions are stored so turns can be updated
    // without checking the map which would generate a sync event and halt the
    // async steps
    uchar is_space_empty_flags = 0b1111;

    ushort wait_move_tick_count = 12;  // 60 FPS / 5 moves per sec = 12
    ushort wait_dig_tick_count = 4;    // 60 FPS / 15 digs per sec = 4

    DualRegisters();
    DualRegisters(const ant_proto::DualRegisters& msg);

    cpu_word_size& operator[](ulong idx);
    cpu_word_size const& operator[](ulong idx) const;

    ant_proto::DualRegisters get_proto() const;
};
