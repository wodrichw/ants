#pragma once

#include <stddef.h>

#include "app/globals.hpp"


using schar = signed char;
using ushort = unsigned short;
using ulong = unsigned long;

class Map;
struct MapEntity;
class Inventory;

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
    cpu_word_size const value = 0;
};

struct MoveOp {
    MoveOp(Map& map, MapEntity& entity, schar dx, schar dy, ulong speed);
    ushort operator()();
    Map& map;
    MapEntity& entity;
    schar dx = '\0', dy = '\0';
    ulong speed = 0;
};

struct DigOp {
    DigOp(Map& map, MapEntity& entity, Inventory& inventory, schar dx, schar dy, ulong speed);
    ushort operator()();
    Map& map;
    MapEntity& entity;
    Inventory& inventory;
    schar dx = '\0', dy = '\0';
    ulong speed = 0 ;
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
    ushort new_idx = 0;
};

struct JnzOp {
    JnzOp(ushort& op_idx, ushort new_idx, bool const& zero_flag);
    ushort operator()();
    ushort& op_idx;
    ushort new_idx = 0;
    bool const& zero_flag;
};
