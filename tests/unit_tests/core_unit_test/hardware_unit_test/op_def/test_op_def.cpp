#include <gtest/gtest.h>

#include "hardware/op_def.hpp"
#include "hardware/brain.hpp"

TEST(OpDefTest, LoadConstantSetsRegisterAndZeroFlag) {
    DualRegisters cpu;
    cpu.zero_flag = true;
    LoadConstantOp op(cpu, cpu[0], 7);
    op();
    EXPECT_EQ(cpu[0], 7u);
    EXPECT_FALSE(cpu.zero_flag);
}

TEST(OpDefTest, CopyOpCopiesRegisters) {
    DualRegisters cpu;
    cpu[0] = 9;
    cpu[1] = 0;
    CopyOp op(cpu, cpu[0], cpu[1]);
    op();
    EXPECT_EQ(cpu[1], 9u);
    EXPECT_FALSE(cpu.zero_flag);
}

TEST(OpDefTest, AddOpAddsRegisters) {
    DualRegisters cpu;
    cpu[0] = 2;
    cpu[1] = 3;
    AddOp op(cpu, cpu[0], cpu[1]);
    op();
    EXPECT_EQ(cpu[1], 5u);
    EXPECT_FALSE(cpu.zero_flag);
}

TEST(OpDefTest, SubOpSubtractsRegisters) {
    DualRegisters cpu;
    cpu[0] = 2;
    cpu[1] = 5;
    SubOp op(cpu, cpu[0], cpu[1]);
    op();
    EXPECT_EQ(cpu[1], 3u);
    EXPECT_FALSE(cpu.zero_flag);
}

TEST(OpDefTest, IncOpIncrementsRegister) {
    DualRegisters cpu;
    cpu[0] = 1;
    IncOp op(cpu, cpu[0]);
    op();
    EXPECT_EQ(cpu[0], 2u);
    EXPECT_FALSE(cpu.zero_flag);
}

TEST(OpDefTest, DecOpDecrementsRegister) {
    DualRegisters cpu;
    cpu[0] = 2;
    DecOp op(cpu, cpu[0]);
    op();
    EXPECT_EQ(cpu[0], 1u);
    EXPECT_FALSE(cpu.zero_flag);
}

TEST(OpDefTest, JmpOpMovesInstructionPointer) {
    DualRegisters cpu;
    cpu.instr_ptr_register = 0;
    JmpOp op(cpu, 5);
    op();
    EXPECT_EQ(cpu.instr_ptr_register, 4);
}

TEST(OpDefTest, JnzOpSkipsWhenZeroFlag) {
    DualRegisters cpu;
    cpu.zero_flag = true;
    cpu.instr_ptr_register = 0;
    JnzOp op(cpu, 5);
    op();
    EXPECT_EQ(cpu.instr_ptr_register, 0);
}

TEST(OpDefTest, JnzOpJumpsWhenZeroFlagClear) {
    DualRegisters cpu;
    cpu.zero_flag = false;
    cpu.instr_ptr_register = 0;
    JnzOp op(cpu, 5);
    op();
    EXPECT_EQ(cpu.instr_ptr_register, 4);
}

TEST(OpDefTest, JnfOpJumpsWhenInstructionNotFailed) {
    DualRegisters cpu;
    cpu.instr_failed_flag = false;
    cpu.instr_ptr_register = 0;
    JnfOp op(cpu, 5);
    op();
    EXPECT_EQ(cpu.instr_ptr_register, 4);
}

TEST(OpDefTest, TurnLeftUpdatesDirectionFlags) {
    DualRegisters cpu;
    cpu.dir_flag1 = false;
    cpu.dir_flag2 = false;
    TurnLeftOp op(cpu);
    op();
    EXPECT_FALSE(cpu.dir_flag1);
    EXPECT_TRUE(cpu.dir_flag2);
}

TEST(OpDefTest, TurnRightUpdatesDirectionFlags) {
    DualRegisters cpu;
    cpu.dir_flag1 = false;
    cpu.dir_flag2 = false;
    TurnRightOp op(cpu);
    op();
    EXPECT_TRUE(cpu.dir_flag1);
    EXPECT_TRUE(cpu.dir_flag2);
}

TEST(OpDefTest, PushAndPopRoundTrip) {
    DualRegisters cpu;
    cpu.stack_ptr_register = 0;
    cpu[0] = 42;

    PushOp push(cpu, cpu[0]);
    push();

    cpu[0] = 0;
    PopOp pop(cpu, cpu[0]);
    pop();

    EXPECT_EQ(cpu[0], 42u);
}

TEST(OpDefTest, MoveOpSetsMoveFlag) {
    DualRegisters cpu;
    cpu.is_move_flag = false;
    MoveOp op(cpu);
    op();
    EXPECT_TRUE(cpu.is_move_flag);
}

TEST(OpDefTest, DigOpSetsDigFlag) {
    DualRegisters cpu;
    cpu.is_dig_flag = false;
    DigOp op(cpu);
    op();
    EXPECT_TRUE(cpu.is_dig_flag);
}
