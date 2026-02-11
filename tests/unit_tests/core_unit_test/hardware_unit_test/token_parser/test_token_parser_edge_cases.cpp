#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "hardware/token_parser.hpp"
#include "utils/status.hpp"

struct TextCase {
    const char* text;
};

static std::vector<TextCase> BuildInvalidIntegerCases() {
    return {
        {""},
        {" "},
        {"foo"},
        {"1.5"},
        {"--1"},
        {"++1"},
        {"-+1"},
        {"0x10"},
        {"NaN"},
        {"inf"},
        {"999999999999999999999"},
        {"-999999999999999999999"},
        {"1_000"},
        {"1,000"},
        {"0b1010"},
        {"0o77"},
        {"A"},
        {"@"},
        {"?"},
        {"/"},
        {"\\"},
        {"."},
        {"-"},
        {"+"},
        {"1e309"}
    };
}

static std::vector<TextCase> BuildInvalidSignedByteCases() {
    return {
        {""},
        {" "},
        {"foo"},
        {"1.5"},
        {"--1"},
        {"0x10"},
        {"NaN"},
        {"999999999999"},
        {"-999999999999"},
        {"1e309"},
        {"A"},
        {"@"},
        {"?"},
        {"-"},
        {"+"}
    };
}

static std::vector<TextCase> BuildInvalidDirectionCases() {
    return {
        {""},
        {"UPP"},
        {"LEFTY"},
        {"DOWNWARD"},
        {"RIGHTS"},
        {"NORTH"},
        {"SOUTH"},
        {"EAST"},
        {"WEST"},
        {"1"}
    };
}

class InvalidIntegerNoThrowTest : public ::testing::TestWithParam<TextCase> {};

TEST_P(InvalidIntegerNoThrowTest, IntegerParserShouldNotThrowOnInvalidInput) {
    auto param = GetParam();
    std::istringstream ss(param.text);
    EXPECT_NO_THROW(TokenParser::integer(ss));
}

INSTANTIATE_TEST_SUITE_P(
    InvalidIntegerCases,
    InvalidIntegerNoThrowTest,
    ::testing::ValuesIn(BuildInvalidIntegerCases())
);

class InvalidSignedByteNoThrowTest
    : public ::testing::TestWithParam<TextCase> {};

TEST_P(InvalidSignedByteNoThrowTest,
       SignedByteParserShouldSetErrorNoThrow) {
    auto param = GetParam();
    std::istringstream ss(param.text);
    Status status;
    EXPECT_NO_THROW(TokenParser::get_signed_byte(ss, status));
    EXPECT_TRUE(status.p_err);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidSignedByteCases,
    InvalidSignedByteNoThrowTest,
    ::testing::ValuesIn(BuildInvalidSignedByteCases())
);

class InvalidDirectionTest : public ::testing::TestWithParam<TextCase> {};

TEST_P(InvalidDirectionTest, DirectionParserSetsErrorOnInvalidInput) {
    auto param = GetParam();
    std::istringstream ss(param.text);
    Status status;
    schar dx = 0;
    schar dy = 0;

    TokenParser::direction(ss, dx, dy, status);

    EXPECT_TRUE(status.p_err);
    EXPECT_EQ(dx, 0);
    EXPECT_EQ(dy, 0);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidDirectionCases,
    InvalidDirectionTest,
    ::testing::ValuesIn(BuildInvalidDirectionCases())
);
