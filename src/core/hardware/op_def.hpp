#pragma once

#include <stddef.h>

#include "app/globals.hpp"


using schar = signed char;
using ushort = unsigned short;
using ulong = unsigned long;

class Map;
struct MapEntity;
class Inventory;

struct Op {
    virtual void operator()()=0;
    virtual unsigned short get_num_ticks() const=0;
    virtual ~Op() {}
};

struct NoOP: Op {
    void operator()() override;
    ushort get_num_ticks() const override;
};

// load a constant to the register
struct LoadConstantOp: Op {
    LoadConstantOp(cpu_word_size& reg, bool& zero_flag, cpu_word_size const value);
    void operator()();
    ushort get_num_ticks() const;

   private:
    cpu_word_size& reg;
    bool& zero_flag;
    cpu_word_size const value = 0;
};

struct MoveOp: Op {
    MoveOp(Map& map, MapEntity& entity, schar dx, schar dy, ulong speed);
    void operator()();
    ushort get_num_ticks() const;
    Map& map;
    MapEntity& entity;
    schar dx = '\0', dy = '\0';
    ulong speed = 0;
};

struct DigOp: Op {
    DigOp(Map& map, MapEntity& entity, Inventory& inventory, schar dx, schar dy, ulong speed);
    void operator()();
    ushort get_num_ticks() const;
    Map& map;
    MapEntity& entity;
    Inventory& inventory;
    schar dx = '\0', dy = '\0';
    ulong speed = 0 ;
};
struct CopyOp: Op {
    CopyOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag);
    void operator()();
    ushort get_num_ticks() const;
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
};

struct AddOp: Op {
    AddOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag);
    void operator()();
    ushort get_num_ticks() const;
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
};

struct SubOp: Op {
    SubOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag);
    void operator()();
    ushort get_num_ticks() const;
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
};

struct IncOp: Op {
    IncOp(cpu_word_size& reg, bool& zero_flag);
    void operator()();
    ushort get_num_ticks() const;
    cpu_word_size& reg;
    bool& zero_flag;
};

struct DecOp: Op {
    DecOp(cpu_word_size& reg, bool& zero_flag);
    void operator()();
    ushort get_num_ticks() const;
    cpu_word_size& reg;
    bool& zero_flag;
};

struct JmpOp: Op {
    JmpOp(ushort& op_idx, ushort new_idx);
    void operator()();
    ushort get_num_ticks() const;
    ushort& op_idx;
    ushort new_idx = 0;
};

struct JnzOp: Op {
    JnzOp(ushort& op_idx, ushort new_idx, bool const& zero_flag);
    void operator()();
    ushort get_num_ticks() const;
    ushort& op_idx;
    ushort new_idx = 0;
    bool const& zero_flag;
};
