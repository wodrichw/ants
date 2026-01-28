#include <gtest/gtest.h>

#include "hardware/brain.hpp"
#include "proto/hardware.pb.h"

TEST(BrainTest, DefaultsToZeroRegisters) {
    DualRegisters cpu;
    EXPECT_EQ(cpu[0], 0u);
    EXPECT_EQ(cpu[1], 0u);
}

TEST(BrainTest, OperatorIndexMutatesRegisters) {
    DualRegisters cpu;
    cpu[0] = 7;
    cpu[1] = 9;
    EXPECT_EQ(cpu[0], 7u);
    EXPECT_EQ(cpu[1], 9u);
}

TEST(BrainTest, ProtoRoundTripRegisters) {
    DualRegisters cpu;
    cpu[0] = 11;
    cpu[1] = 22;

    auto msg = cpu.get_proto();
    DualRegisters restored(msg);

    EXPECT_EQ(restored[0], 11u);
    EXPECT_EQ(restored[1], 22u);
}

TEST(BrainTest, ProtoRoundTripFlags) {
    DualRegisters cpu;
    cpu.zero_flag = true;
    cpu.dir_flag1 = true;
    cpu.dir_flag2 = false;
    cpu.is_move_flag = true;
    cpu.is_dig_flag = true;
    cpu.instr_failed_flag = true;

    auto msg = cpu.get_proto();
    DualRegisters restored(msg);

    EXPECT_TRUE(restored.zero_flag);
    EXPECT_TRUE(restored.dir_flag1);
    EXPECT_FALSE(restored.dir_flag2);
    EXPECT_TRUE(restored.is_move_flag);
    EXPECT_TRUE(restored.is_dig_flag);
    EXPECT_TRUE(restored.instr_failed_flag);
}

TEST(BrainTest, InstructionPointersRoundTrip) {
    DualRegisters cpu;
    cpu.instr_ptr_register = 3;
    cpu.base_ptr_register = 4;
    cpu.stack_ptr_register = 5;

    auto msg = cpu.get_proto();
    DualRegisters restored(msg);

    EXPECT_EQ(restored.instr_ptr_register, 3);
    EXPECT_EQ(restored.base_ptr_register, 4);
    EXPECT_EQ(restored.stack_ptr_register, 5);
}

TEST(BrainTest, RamCanStoreValues) {
    DualRegisters cpu;
    cpu.ram[0] = 99;
    EXPECT_EQ(cpu.ram[0], 99u);
}

TEST(BrainTest, DirectionFlagsDefaultToRight) {
    DualRegisters cpu;
    EXPECT_FALSE(cpu.dir_flag1);
    EXPECT_FALSE(cpu.dir_flag2);
}

TEST(BrainTest, MoveAndDigFlagsDefaultFalse) {
    DualRegisters cpu;
    EXPECT_FALSE(cpu.is_move_flag);
    EXPECT_FALSE(cpu.is_dig_flag);
}

TEST(BrainTest, RegistersPersistThroughProto) {
    DualRegisters cpu;
    cpu[0] = 123;
    cpu[1] = 456;

    ant_proto::DualRegisters msg = cpu.get_proto();
    DualRegisters restored(msg);

    EXPECT_EQ(restored[0], 123u);
    EXPECT_EQ(restored[1], 456u);
}

TEST(BrainTest, ZeroFlagPersistsThroughProto) {
    DualRegisters cpu;
    cpu.zero_flag = true;

    auto msg = cpu.get_proto();
    DualRegisters restored(msg);

    EXPECT_TRUE(restored.zero_flag);
}
