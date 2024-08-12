#include "hardware/op_def.hpp"
#include "entity/map.hpp"
#include "entity/entity_actions.hpp"
#include "hardware/brain.hpp"

#include "spdlog/spdlog.h"

// NOP //////////////////////////////////////////
NoOP::NoOP(DualRegisters&) {}
void NoOP::operator()() {
    SPDLOG_TRACE("NOP operation executed");
}

// LOAD CONSTANT TO REGISTER ////////////////////
LoadConstantOp::LoadConstantOp(DualRegisters& cpu, cpu_word_size& reg, cpu_word_size const value)
    : reg(reg), zero_flag(cpu.zero_flag), value(value) {
        SPDLOG_DEBUG("LoadConstantOp created");
    }
void LoadConstantOp::operator()() {
    SPDLOG_DEBUG("Executing LoadConstantOp");
    SPDLOG_TRACE("Writing value {} to register", value);
    reg = value;
    zero_flag = value == 0;
}

// MOVE /////////////////////////////////////////
MoveOp::MoveOp(DualRegisters& cpu)
    : is_move_flag(cpu.is_move_flag) {
        SPDLOG_DEBUG("MoveOp created");
    }
void MoveOp::operator()() {
    SPDLOG_DEBUG("Executing MoveOp");
    is_move_flag = true;
}

// DIG /////////////////////////////////////////
DigOp::DigOp(DualRegisters& cpu)
    : is_dig_flag(cpu.is_dig_flag) {
        SPDLOG_DEBUG("DigOp created");
    }
void DigOp::operator()() {
    SPDLOG_DEBUG("Executing DigOp");
    is_dig_flag = true;
}

// COPY REGISTER TO REGISTER ////////////////////
CopyOp::CopyOp(DualRegisters& cpu, cpu_word_size& reg_src, cpu_word_size& reg_dst)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(cpu.zero_flag) {
        SPDLOG_DEBUG("CopyOp created");
      }
void CopyOp::operator()() {
    SPDLOG_DEBUG("Executing CopyOp");
    reg_dst = reg_src;
    zero_flag = reg_src == 0;
    SPDLOG_TRACE("Copying register with result {}", reg_dst);
}

// ADD REGISTER TO REGISTER ////////////////////
AddOp::AddOp(DualRegisters& cpu, cpu_word_size& reg_src, cpu_word_size& reg_dst)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(cpu.zero_flag) {
        SPDLOG_TRACE("AddOp created");
      }
void AddOp::operator()() {
    SPDLOG_DEBUG("Executing AddOp");
    reg_dst += reg_src;
    zero_flag = reg_dst == 0;
    SPDLOG_TRACE("Adding registers - result: {}", reg_dst);
}

// SUB REGISTER TO REGISTER ////////////////////
SubOp::SubOp(DualRegisters& cpu, cpu_word_size& reg_src, cpu_word_size& reg_dst)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(cpu.zero_flag) {
        SPDLOG_DEBUG("SubOp created");
      }
void SubOp::operator()() {
    SPDLOG_DEBUG("Executing SubOp");
    reg_dst -= reg_src;
    zero_flag = reg_dst == 0;
    SPDLOG_TRACE("Subtracting registers - result: {}", reg_dst);
}

// INC REGISTER ////////////////////////////////
IncOp::IncOp(DualRegisters& cpu, cpu_word_size& reg)
    : reg(reg), zero_flag(cpu.zero_flag) {
        SPDLOG_DEBUG("IncOp created");
    }
void IncOp::operator()() {
    SPDLOG_DEBUG("Executing IncOp");
    ++reg;
    zero_flag = reg == 0;
    SPDLOG_TRACE("Incremented register - result: {} - zero flag: {}", reg, zero_flag);
}

// DEC REGISTER ////////////////////////////////
DecOp::DecOp(DualRegisters& cpu, cpu_word_size& reg)
    : reg(reg), zero_flag(cpu.zero_flag) {
        SPDLOG_DEBUG("DecOp created");
    }
void DecOp::operator()() {
    SPDLOG_DEBUG("Executing DecOp");
    --reg;
    zero_flag = reg == 0;
    SPDLOG_TRACE("Decremented register - result: {} - zero flag: {}", reg, zero_flag);
}

// JMP /////////////////////////////////////////
JmpOp::JmpOp(DualRegisters& cpu, ushort address)
    : instr_ptr_register(cpu.instr_ptr_register), address(address-1) {
        SPDLOG_DEBUG("JmpOp created - jumping to: {}", address);
    }

void JmpOp::operator()() {
    SPDLOG_DEBUG("Executing JmpOp");
    instr_ptr_register = address;
    SPDLOG_TRACE("Jumped to address {}", address);
}

// JNZ /////////////////////////////////////////
JnzOp::JnzOp(DualRegisters& cpu, ushort address)
    : instr_ptr_register(cpu.instr_ptr_register), address(address - 1),
      zero_flag(cpu.zero_flag) {
        SPDLOG_DEBUG("JnzOp created - jumping to: {}", address);
    }

void JnzOp::operator()() {
    SPDLOG_DEBUG("Executing JnzOp");
    if(zero_flag){
        SPDLOG_TRACE("Zero flag is set, not jumping");
        return;
    }
    SPDLOG_TRACE("Zero flag off - jumping to address {}", address);
    instr_ptr_register = address;
}

// JNF /////////////////////////////////////////
JnfOp::JnfOp(DualRegisters& cpu, ushort address)
    : instr_ptr_register(cpu.instr_ptr_register), address(address - 1),
      instr_failed_flag(cpu.instr_failed_flag) {
        SPDLOG_DEBUG("JnfOp created - jumping to: {}", address);
    }

void JnfOp::operator()() {
    SPDLOG_DEBUG("Executing JnfOp");
    if(instr_failed_flag){
        SPDLOG_TRACE("Instruction failed flag is set, not jumping");
        return;
    }
    SPDLOG_TRACE("Instruction failed flag off - jumping to address {}", address);
    instr_ptr_register = address;
}

// CALL /////////////////////////////////////////
CallOp::CallOp(DualRegisters& cpu, ushort address)
    : ram(cpu.ram),
      instr_ptr_register(cpu.instr_ptr_register),
      base_ptr_register(cpu.base_ptr_register),
      stack_ptr_register(cpu.stack_ptr_register),
      address(address - 1) {
        SPDLOG_DEBUG("Call operation created - jumping to: {}", address);
    }

void CallOp::operator()() {
    SPDLOG_TRACE("Executing call operation - jumping to address {}", address);

    // Calling a function
    // Reversed by 'returning' in the function call

    // Push the instruction pointer register to the stack
    ram[stack_ptr_register] = instr_ptr_register;
    ++stack_ptr_register;

    // Pushing base pointer register to the stack
    ram[stack_ptr_register] = base_ptr_register;
    ++stack_ptr_register;

    // Move the instruction pointer
    instr_ptr_register = address;

    // Set the base pointer register
    base_ptr_register = stack_ptr_register;
}

// TURN LEFT /////////////////////////////////////////
TurnLeftOp::TurnLeftOp(DualRegisters& cpu)
    : dir_flag1(cpu.dir_flag1),
      dir_flag2(cpu.dir_flag2) {
        SPDLOG_DEBUG("Turn left operation created");
    }

void TurnLeftOp::operator()() {
    SPDLOG_TRACE("Executing turn left operation");

    // Truth Table - Counterclockwise rotation

    // X Y   A B
    // 0 0 | 0 1
    // 0 1 | 1 0
    // 1 0 | 1 1
    // 1 1 | 0 0

    // A = X ^ Y
    // B = !Y

    // Update heading
    dir_flag1 = dir_flag1 != dir_flag2;
    dir_flag2 = !dir_flag2;
}

// TURN RIGHT /////////////////////////////////////////
TurnRightOp::TurnRightOp(DualRegisters& cpu)
    : dir_flag1(cpu.dir_flag1),
      dir_flag2(cpu.dir_flag2) {
        SPDLOG_DEBUG("Turn right operation created");
    }

void TurnRightOp::operator()() {
    SPDLOG_TRACE("Executing turn right operation");

    // Truth Table

    // X Y   A B
    // 0 0 | 1 1
    // 0 1 | 0 0
    // 1 0 | 0 1
    // 1 1 | 1 0

    // A = !(X ^ Y)
    // B = ! Y

    // Update heading
    dir_flag1 = dir_flag1 == dir_flag2;
    dir_flag2 = !dir_flag2;
}

// POP REGISTER TO RAM ////////////////////////////////
PopOp::PopOp(DualRegisters& cpu, cpu_word_size& reg)
    : ram(cpu.ram), reg(reg), stack_ptr_register(cpu.stack_ptr_register) {
        SPDLOG_DEBUG("Pop operation created");
    }
void PopOp::operator()() {
    SPDLOG_DEBUG("Executing pop");
    reg = ram[--stack_ptr_register];
}

// POP REGISTER TO RAM ////////////////////////////////
PushOp::PushOp(DualRegisters& cpu, cpu_word_size& reg)
    : ram(cpu.ram), reg(reg), stack_ptr_register(cpu.stack_ptr_register) {
        SPDLOG_DEBUG("Push operation created");
    }
void PushOp::operator()() {
    SPDLOG_DEBUG("Executing push");
    ram[stack_ptr_register] = reg;
    ++stack_ptr_register;
}

// RETURN FROM FUNCTION /////////////////////////////////////////
ReturnOp::ReturnOp(DualRegisters& cpu)
    : ram(cpu.ram),
      instr_ptr_register(cpu.instr_ptr_register),
      base_ptr_register(cpu.base_ptr_register),
      stack_ptr_register(cpu.stack_ptr_register) {
        SPDLOG_DEBUG("Return operation created");
    }

void ReturnOp::operator()() {
    SPDLOG_TRACE("Executing return operation");

    // Restore the stack pointer
    stack_ptr_register = base_ptr_register;

    // Restore the base pointer
    base_ptr_register = ram[--stack_ptr_register];

    // Get the previous instruction address
    instr_ptr_register = ram[--stack_ptr_register];
}

// CHECK MAP /////////////////////////////////////////
CheckOp::CheckOp(DualRegisters& cpu)
    : dir_flag1(cpu.dir_flag1),
      dir_flag2(cpu.dir_flag2),
      is_space_empty_flags(cpu.is_space_empty_flags),
      instr_failed_flag(cpu.instr_failed_flag) {
        SPDLOG_DEBUG("Check operation created");
    }

void CheckOp::operator()() {
    SPDLOG_TRACE("Executing check operation");

    // Truth Table
    // X Y   Index
    // 0 0 | 0
    // 0 1 | 1
    // 1 0 | 2
    // 1 1 | 3

    uchar idx = (dir_flag1 << 1) | dir_flag2;
    bool is_empty = (is_space_empty_flags >> idx) & 1;
    instr_failed_flag = !is_empty;
    SPDLOG_TRACE("Checking direction: {} -> {}", "RULD"[idx], (is_empty ? "EMPTY": "FULL"));
}

// SCENT ON /////////////////////////////////////////
ScentOnOp::ScentOnOp(DualRegisters& cpu, uchar scent_idx)
    : scent_behaviors(cpu.scent_behaviors), delta_scents(cpu.delta_scents),
      scent_idx(scent_idx) {}

void ScentOnOp::operator()() {
    scent_behaviors.write_scent_behavior = IncrementScentBehavior(delta_scents, scent_idx);
}

// SCENT OFF /////////////////////////////////////////
ScentOffOp::ScentOffOp(DualRegisters& cpu)
    : scent_behaviors(cpu.scent_behaviors) {}

void ScentOffOp::operator()() {
    scent_behaviors.write_scent_behavior = ImmutableScentBehavior();
}

// SET SCENT PRIORITY /////////////////////////////////////////
SetScentPriorityOp::SetScentPriorityOp(DualRegisters& cpu, uchar scent_idx, uchar priority)
    : priorities(cpu.scent_behaviors.priorities),
      clear_mask(~(0xFFUL << (scent_idx * 8))),
      priority(static_cast<ulong>(priority) << (scent_idx * 8)) {}

void SetScentPriorityOp::operator()() {
    priorities = (priorities & clear_mask) | priority;
}

// TURN DIRECTION BY READING SCENT /////////////////////////////////////////
TurnByScentOp::TurnByScentOp(DualRegisters& cpu)
    : dir_flag1(cpu.dir_flag1), dir_flag2(cpu.dir_flag2),
      scent_dir1(cpu.scent_behaviors.scent_dir1),
      scent_dir2(cpu.scent_behaviors.scent_dir2) {}

void TurnByScentOp::operator()() {
    dir_flag1 = scent_dir1;
    dir_flag2 = scent_dir2;
}