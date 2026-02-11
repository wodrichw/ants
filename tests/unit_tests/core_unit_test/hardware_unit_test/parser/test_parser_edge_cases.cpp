#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "hardware/command_config.hpp"
#include "hardware/machine_code.hpp"
#include "hardware/parser.hpp"
#include "utils/status.hpp"

struct InvalidProgramCase {
    std::vector<std::string> lines;
};

static std::vector<InvalidProgramCase> BuildInvalidProgramCases() {
    std::vector<InvalidProgramCase> cases;

    const std::vector<std::string> invalid_register_tokens = {
        "1", "a", "aa", "!", "@", "#", "?", "-"};
    const std::vector<std::string> invalid_two_reg_tokens = {
        "1", "a", "?", "!", "@", "Z1"};
    const std::vector<std::string> invalid_load_values = {
        "", "foo", "1.5", "99999999999999999999", "--1",
        "0x10", "NaN", "+", "-", " "};
    const std::vector<std::string> invalid_load_registers = {"1", "a", "*",
                                                             "?"};
    const std::vector<std::string> invalid_jump_labels = {
        "missing", "", ":", "1", "a", "label-with-dash", "L1"};
    const std::vector<std::string> invalid_swp_priorities = {"foo", "9999999",
                                                             "+", "--1"};
    const std::vector<std::string> invalid_swn_tokens = {"1", "a", "?", "@",
                                                         "AA", "-"};

    const std::vector<std::string> one_reg_cmds = {"INC", "DEC", "POP",
                                                   "PUSH"};
    for(const auto& cmd : one_reg_cmds) {
        for(const auto& tok : invalid_register_tokens) {
            cases.push_back({{cmd + " " + tok}});
        }
    }

    const std::vector<std::string> two_reg_cmds = {"COPY", "ADD", "SUB"};
    for(const auto& cmd : two_reg_cmds) {
        for(const auto& tok : invalid_two_reg_tokens) {
            cases.push_back({{cmd + " " + tok + " A"}});
        }
    }
    for(const auto& cmd : two_reg_cmds) {
        for(const auto& tok : invalid_two_reg_tokens) {
            cases.push_back({{cmd + " A " + tok}});
        }
    }

    for(const auto& tok : invalid_load_registers) {
        cases.push_back({{"LOAD " + tok + " 1"}});
    }
    for(const auto& tok : invalid_load_values) {
        std::string line = "LOAD A";
        if(!tok.empty()) line += " " + tok;
        cases.push_back({{line}});
    }

    const std::vector<std::string> jump_cmds = {"JMP", "JNZ", "JNF", "CALL"};
    for(const auto& cmd : jump_cmds) {
        for(const auto& tok : invalid_jump_labels) {
            if(tok.empty()) {
                cases.push_back({{cmd}});
            } else {
                cases.push_back({{cmd + " " + tok}});
            }
        }
    }

    for(const auto& tok : invalid_swp_priorities) {
        cases.push_back({{"SWP A " + tok}});
    }

    for(const auto& tok : invalid_swn_tokens) {
        cases.push_back({{"SWN " + tok}});
    }

    const std::vector<std::string> extra_args = {
        "MOVE X", "DIG 1", "NOP A", "LT Z", "RT 1",
        "RET 0",  "CHK B", "SRT 9", "SWF X", "MOVE A"};
    for(const auto& line : extra_args) {
        cases.push_back({{line}});
    }

    return cases;
}

class ParserInvalidProgramTest
    : public ::testing::TestWithParam<InvalidProgramCase> {};

TEST_P(ParserInvalidProgramTest, InvalidProgramsShouldSetErrorNoThrow) {
    CommandMap command_map;
    Parser parser(command_map);
    MachineCode machine_code;
    Status status;

    auto param = GetParam();
    EXPECT_NO_THROW(parser.parse(param.lines, machine_code, status));
    EXPECT_TRUE(status.p_err);
}

INSTANTIATE_TEST_SUITE_P(
    ParserInvalidPrograms,
    ParserInvalidProgramTest,
    ::testing::ValuesIn(BuildInvalidProgramCases())
);
