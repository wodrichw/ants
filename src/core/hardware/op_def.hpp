#pragma once

#include <stddef.h>

#include "app/globals.hpp"
#include "entity/map.hpp"
#include "entity/entity_data.hpp"
struct NoOP {
    bool operator()();
};

// load a constant to the register
struct LoadConstantOp {
    LoadConstantOp(cpu_word_size& reg, bool& zero_flag, cpu_word_size const value);
    bool operator()();

   private:
    cpu_word_size& reg;
    bool& zero_flag;
    cpu_word_size const value;
};

struct MoveOp {
    MoveOp(Map& map, MapEntity& entity, long dx, long dy);
    bool operator()();
    Map& map;
    MapEntity& entity;
    long dx, dy;
};

struct CopyOp {
    CopyOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag);
    bool operator()();
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
};

struct AddOp {
    AddOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag);
    bool operator()();
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
};

struct SubOp {
    SubOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag);
    bool operator()();
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
};

struct IncOp {
    IncOp(cpu_word_size& reg, bool& zero_flag);
    bool operator()();
    cpu_word_size& reg;
    bool& zero_flag;
};

struct DecOp {
    DecOp(cpu_word_size& reg, bool& zero_flag);
    bool operator()();
    cpu_word_size& reg;
    bool& zero_flag;
};

struct JmpOp {
    JmpOp(size_t& op_idx, size_t new_idx);
    bool operator()();
    size_t& op_idx;
    size_t new_idx;
};

struct JnzOp {
    JnzOp(size_t& op_idx, size_t new_idx, bool const& zero_flag);
    bool operator()();
    size_t& op_idx;
    size_t new_idx;
    bool const& zero_flag;
};
