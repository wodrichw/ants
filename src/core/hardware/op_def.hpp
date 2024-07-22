#pragma once

#include <stddef.h>

#include "app/globals.hpp"


using schar = signed char;
using ushort = unsigned short;
using ulong = unsigned long;

struct DualRegisters;

struct NoOP {
    NoOP(DualRegisters&);
    void operator()();
};

// load a constant to the register
struct LoadConstantOp {
    LoadConstantOp(DualRegisters&, cpu_word_size& reg, cpu_word_size const value);
    void operator()();

   private:
    cpu_word_size& reg;
    bool& zero_flag;
    bool& instr_failed_flag;
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
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
    bool& instr_failed_flag;
};

struct AddOp {
    AddOp(DualRegisters&, cpu_word_size& reg_src, cpu_word_size& reg_dst);
    void operator()();
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
    bool& instr_failed_flag;
};

struct SubOp {
    SubOp(DualRegisters&, cpu_word_size& reg_src, cpu_word_size& reg_dst);
    void operator()();
    cpu_word_size& reg_src, &reg_dst;
    bool& zero_flag;
    bool& instr_failed_flag;
};

struct IncOp {
    IncOp(DualRegisters&, cpu_word_size& reg);
    void operator()();
    cpu_word_size& reg;
    bool& zero_flag;
    bool& instr_failed_flag;
};

struct DecOp {
    DecOp(DualRegisters&, cpu_word_size& reg);
    void operator()();
    cpu_word_size& reg;
    bool& zero_flag;
    bool& instr_failed_flag;
};

struct JmpOp {
    JmpOp(DualRegisters&, ushort address);
    void operator()();
    ushort& instr_ptr_register;
    ushort address = 0;
    bool& instr_failed_flag;
};

struct JnzOp {
    JnzOp(DualRegisters&, ushort address);
    void operator()();
    ushort& instr_ptr_register;
    ushort address = 0;
    bool const& zero_flag;
    bool& instr_failed_flag;
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
    bool& instr_failed_flag;

    ushort address = 0;
};

struct TurnLeftOp {
    TurnLeftOp(DualRegisters&);
    void operator()();
    bool& dir_flag1;
    bool& dir_flag2;
    bool& instr_failed_flag;
};

struct TurnRightOp {
    TurnRightOp(DualRegisters&);
    void operator()();
    bool& dir_flag1;
    bool& dir_flag2;
    bool& instr_failed_flag;
};

struct PopOp {
    PopOp(DualRegisters&, cpu_word_size& reg);
    void operator()();

    cpu_word_size* ram;
    cpu_word_size& reg;
    ushort& stack_ptr_register;
    bool& instr_failed_flag;
};

struct PushOp {
    PushOp(DualRegisters&, cpu_word_size& reg);
    void operator()();

    cpu_word_size* ram;
    cpu_word_size& reg;
    ushort& stack_ptr_register;
    bool& instr_failed_flag;
};

struct ReturnOp {
    ReturnOp(DualRegisters&);
    void operator()();

    cpu_word_size* ram;
    ushort& instr_ptr_register;
    ushort& base_ptr_register;
    ushort& stack_ptr_register;
    bool& instr_failed_flag;
};