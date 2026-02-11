#include <gtest/gtest.h>

#include "hardware/command_config.hpp"
#include "hardware/machine_code.hpp"
#include "hardware/parser.hpp"
#include "utils/status.hpp"

struct ParseCase {
    std::vector<std::string> lines;
    size_t expected_code_size;
};

class ParserParseTest : public ::testing::TestWithParam<ParseCase> {};

TEST_P(ParserParseTest, ParsesProgramsIntoMachineCode) {
    CommandMap command_map;
    Parser parser(command_map);
    MachineCode machine_code;
    Status status;

    auto param = GetParam();
    parser.parse(param.lines, machine_code, status);

    EXPECT_FALSE(status.p_err);
    EXPECT_EQ(machine_code.size(), param.expected_code_size);
}

INSTANTIATE_TEST_SUITE_P(
    ParserParseCases,
    ParserParseTest,
    ::testing::Values(
        ParseCase{{"NOP"}, 1},
        ParseCase{{"LOAD A 1"}, 5},
        ParseCase{{"INC A"}, 1},
        ParseCase{{"start:", "JMP start"}, 3},
        ParseCase{{"MOVE"}, 1}
    )
);

struct DeparseCase {
    std::vector<uchar> code;
    std::string expected_line;
};

class ParserDeparseTest : public ::testing::TestWithParam<DeparseCase> {};

TEST_P(ParserDeparseTest, DeparsesMachineCodeIntoLines) {
    CommandMap command_map;
    Parser parser(command_map);
    Status status;
    std::vector<std::string> lines;
    MachineCode machine_code;

    auto param = GetParam();
    machine_code.code = param.code;

    parser.deparse(machine_code, lines, status);

    EXPECT_FALSE(status.p_err);
    ASSERT_FALSE(lines.empty());
    EXPECT_EQ(lines[0], param.expected_line);
}

INSTANTIATE_TEST_SUITE_P(
    ParserDeparseCases,
    ParserDeparseTest,
    ::testing::Values(
        DeparseCase{{static_cast<uchar>(CommandEnum::NOP << 3)}, "NOP"},
        DeparseCase{{static_cast<uchar>((CommandEnum::INC << 3) | 0)}, "INC A"},
        DeparseCase{{static_cast<uchar>((CommandEnum::DEC << 3) | 1)}, "DEC B"},
        DeparseCase{{static_cast<uchar>(CommandEnum::MOVE << 3)}, "MOVE"},
        DeparseCase{{static_cast<uchar>((CommandEnum::LOAD << 3) | 1), 5, 0, 0, 0}, "LOAD B 5"}
    )
);
