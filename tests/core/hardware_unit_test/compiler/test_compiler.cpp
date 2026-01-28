#include <gtest/gtest.h>

#include "hardware/command_config.hpp"
#include "hardware/compiler.hpp"
#include "hardware/compile_args.hpp"
#include "hardware/brain.hpp"
#include "hardware/program_executor.hpp"

struct CompileCase {
    CommandEnum cmd;
};

class CompilerTest : public ::testing::TestWithParam<CompileCase> {};

TEST_P(CompilerTest, CompilesNoArgInstructions) {
    CommandMap map;
    Compiler compiler(map);
    DualRegisters cpu;
    std::vector<Op> ops;

    auto param = GetParam();
    std::vector<uchar> code = {
        static_cast<uchar>(param.cmd << 3)
    };

    CompileArgs args(code, cpu, ops);
    compiler.compile(args);

    EXPECT_EQ(ops.size(), 1u);
    EXPECT_EQ(args.code_it, code.end());
}

INSTANTIATE_TEST_SUITE_P(
    CompilerCases,
    CompilerTest,
    ::testing::Values(
        CompileCase{CommandEnum::NOP},
        CompileCase{CommandEnum::MOVE},
        CompileCase{CommandEnum::DIG},
        CompileCase{CommandEnum::LT},
        CompileCase{CommandEnum::RT}
    )
);
