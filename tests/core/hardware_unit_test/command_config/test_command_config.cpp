#include <gtest/gtest.h>

#include "hardware/command_config.hpp"

struct CommandCase {
    CommandEnum cmd;
    const char* name;
};

class CommandConfigTest : public ::testing::TestWithParam<CommandCase> {};

TEST_P(CommandConfigTest, CommandMapContainsCommand) {
    CommandMap map;
    auto param = GetParam();

    auto it = map.find(param.name);
    EXPECT_NE(it, map.str_end());
    EXPECT_EQ(map.at(param.cmd).command_string, param.name);
}

INSTANTIATE_TEST_SUITE_P(
    CommandCases,
    CommandConfigTest,
    ::testing::Values(
        CommandCase{CommandEnum::NOP, "NOP"},
        CommandCase{CommandEnum::LOAD, "LOAD"},
        CommandCase{CommandEnum::MOVE, "MOVE"},
        CommandCase{CommandEnum::JMP, "JMP"},
        CommandCase{CommandEnum::SCENT_ON, "SWN"}
    )
);
