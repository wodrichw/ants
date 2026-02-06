#pragma once

#include <stddef.h>

#include "app/globals.hpp"
#include "entity/scents.hpp"
#include "utils/types.hpp"

using uchar = unsigned char;
using schar = signed char;

struct DualRegisters;

struct NoOP {
    NoOP(DualRegisters&);
    void operator()();
};

// load a constant to the register
struct LoadConstantOp {
    LoadConstantOp(DualRegisters&, cpu_word_size& reg,
                   cpu_word_size const value);
    void operator()();

   private:
    cpu_word_size& reg;
    bool& zero_flag;
    cpu_word_size const value = 0;
};

struct MoveOp {
    MoveOp(DualRegisters&);
    void operator()();
    bool& is_move_flag;
};

struct DigOp {
    DigOp(DualRegisters&);
    void operator()();
    bool& is_dig_flag;
};
struct CopyOp {
    CopyOp(DualRegisters&, cpu_word_size& reg_src, cpu_word_size& reg_dst);
    void operator()();
    cpu_word_size &reg_src, &reg_dst;
    bool& zero_flag;
};

struct AddOp {
    AddOp(DualRegisters&, cpu_word_size& reg_src, cpu_word_size& reg_dst);
    void operator()();
    cpu_word_size &reg_src, &reg_dst;
    bool& zero_flag;
};

struct SubOp {
    SubOp(DualRegisters&, cpu_word_size& reg_src, cpu_word_size& reg_dst);
    void operator()();
    cpu_word_size &reg_src, &reg_dst;
    bool& zero_flag;
};

struct IncOp {
    IncOp(DualRegisters&, cpu_word_size& reg);
    void operator()();
    cpu_word_size& reg;
    bool& zero_flag;
};

struct DecOp {
    DecOp(DualRegisters&, cpu_word_size& reg);
    void operator()();
    cpu_word_size& reg;
    bool& zero_flag;
};

struct JmpOp {
    JmpOp(DualRegisters&, ushort address);
    void operator()();
    ushort& instr_ptr_register;
    ushort address = 0;
};

struct JnzOp {
    JnzOp(DualRegisters&, ushort address);
    void operator()();
    ushort& instr_ptr_register;
    ushort address = 0;
    bool const& zero_flag;
};

struct JnfOp {
    JnfOp(DualRegisters&, ushort address);
    void operator()();
    ushort& instr_ptr_register;
    ushort address = 0;
    bool& instr_failed_flag;
};

struct CallOp {
    CallOp(DualRegisters&, ushort address);
    void operator()();

    cpu_word_size* ram;
    ushort& instr_ptr_register;
    ushort& base_ptr_register;
    ushort& stack_ptr_register;

    ushort address = 0;
};

struct TurnLeftOp {
    TurnLeftOp(DualRegisters&);
    void operator()();
    bool& dir_flag1;
    bool& dir_flag2;
};

struct TurnRightOp {
    TurnRightOp(DualRegisters&);
    void operator()();
    bool& dir_flag1;
    bool& dir_flag2;
};

struct PopOp {
    PopOp(DualRegisters&, cpu_word_size& reg);
    void operator()();

    cpu_word_size* ram;
    cpu_word_size& reg;
    ushort& stack_ptr_register;
};

struct PushOp {
    PushOp(DualRegisters&, cpu_word_size& reg);
    void operator()();

    cpu_word_size* ram;
    cpu_word_size& reg;
    ushort& stack_ptr_register;
};

struct ReturnOp {
    ReturnOp(DualRegisters&);
    void operator()();

    cpu_word_size* ram;
    ushort& instr_ptr_register;
    ushort& base_ptr_register;
    ushort& stack_ptr_register;
};

struct CheckOp {
    CheckOp(DualRegisters&);
    void operator()();

    bool& dir_flag1;
    bool& dir_flag2;
    uchar& is_space_empty_flags;
    bool& instr_failed_flag;
};

struct ScentOnOp {
    ScentBehaviors& scent_behaviors;
    ulong& delta_scents;
    uchar scent_idx;
    ScentOnOp(DualRegisters&, uchar);
    void operator()();
};

struct ScentOffOp {
    ScentBehaviors& scent_behaviors;
    ScentOffOp(DualRegisters&);
    void operator()();
};

struct SetScentPriorityOp {
    ulong& priorities;
    ulong clear_mask, priority;
    SetScentPriorityOp(DualRegisters&, uchar scent_idx, uchar priority);
    void operator()();
};

struct TurnByScentOp {
    bool &dir_flag1, &dir_flag2;
    bool &scent_dir1, &scent_dir2;
    TurnByScentOp(DualRegisters&);
    void operator()();
};
