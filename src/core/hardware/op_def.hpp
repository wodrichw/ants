#pragma once

#include <stddef.h>

#include "app/globals.hpp"
#include "entity/map.hpp"
#include "entity/entity_data.hpp"

using schar = signed char;

struct NoOP {
    ushort operator()();
};

// load a constant to the register
struct LoadConstantOp {
    LoadConstantOp(cpu_word_size& reg, bool& zero_flag, cpu_word_size const value);
    ushort operator()();

   private:
    cpu_word_size& reg;
    bool& zero_flag;
    cpu_word_size const value;
};

struct MoveOp {
    MoveOp(Map& map, MapEntity& entity, schar dx, schar dy, ulong speed);
    ushort operator()();
    Map& map;
    MapEntity& entity;
    schar dx, dy;
    ulong speed;
};

struct CopyOp {
    CopyOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag);
    ushort operator()();
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
};

struct AddOp {
    AddOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag);
    ushort operator()();
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
};

struct SubOp {
    SubOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag);
    ushort operator()();
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
};

struct IncOp {
    IncOp(cpu_word_size& reg, bool& zero_flag);
    ushort operator()();
    cpu_word_size& reg;
    bool& zero_flag;
};

struct DecOp {
    DecOp(cpu_word_size& reg, bool& zero_flag);
    ushort operator()();
    cpu_word_size& reg;
    bool& zero_flag;
};

struct JmpOp {
    JmpOp(ushort& op_idx, ushort new_idx);
    ushort operator()();
    ushort& op_idx;
    ushort new_idx;
};

struct JnzOp {
    JnzOp(ushort& op_idx, ushort new_idx, bool const& zero_flag);
    ushort operator()();
    ushort& op_idx;
    ushort new_idx;
    bool const& zero_flag;
};
