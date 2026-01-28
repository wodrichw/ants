#include <gtest/gtest.h>

#include "hardware/hardware_manager.hpp"
#include "hardware/command_config.hpp"
#include "hardware/compile_args.hpp"
#include "hardware/program_executor.hpp"
#include "utils/thread_pool.hpp"

TEST(HardwareManagerTest, StartsWithEmptyExecutorList) {
    CommandMap map;
    HardwareManager manager(map);
    EXPECT_EQ(std::distance(manager.begin(), manager.end()), 0);
}

TEST(HardwareManagerTest, PushBackAddsExecutor) {
    CommandMap map;
    HardwareManager manager(map);

    ulong instr_clock = 0;
    ushort instr_ptr = 0;
    ThreadPool<AsyncProgramJob> pool(1);
    ProgramExecutor exec(instr_clock, 1, instr_ptr, pool);

    manager.push_back(&exec);

    EXPECT_EQ(std::distance(manager.begin(), manager.end()), 1);
}

TEST(HardwareManagerTest, CompileGeneratesSingleOp) {
    CommandMap map;
    HardwareManager manager(map);
    DualRegisters cpu;
    std::vector<Op> ops;
    std::vector<uchar> code = {static_cast<uchar>(CommandEnum::NOP << 3)};

    CompileArgs args(code, cpu, ops);
    manager.compile(args);

    EXPECT_EQ(ops.size(), 1u);
}

TEST(HardwareManagerTest, CompileGeneratesMultipleOps) {
    CommandMap map;
    HardwareManager manager(map);
    DualRegisters cpu;
    std::vector<Op> ops;
    std::vector<uchar> code = {
        static_cast<uchar>(CommandEnum::NOP << 3),
        static_cast<uchar>(CommandEnum::MOVE << 3)
    };

    CompileArgs args(code, cpu, ops);
    manager.compile(args);

    EXPECT_EQ(ops.size(), 2u);
}

TEST(HardwareManagerTest, PushBackSupportsMultipleExecutors) {
    CommandMap map;
    HardwareManager manager(map);

    ulong instr_clock = 0;
    ushort instr_ptr_a = 0;
    ushort instr_ptr_b = 0;
    ThreadPool<AsyncProgramJob> pool(1);

    ProgramExecutor exec_a(instr_clock, 1, instr_ptr_a, pool);
    ProgramExecutor exec_b(instr_clock, 1, instr_ptr_b, pool);

    manager.push_back(&exec_a);
    manager.push_back(&exec_b);

    EXPECT_EQ(std::distance(manager.begin(), manager.end()), 2);
}
