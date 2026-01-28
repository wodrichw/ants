#include <gtest/gtest.h>

#include "hardware/token_parser.hpp"
#include "utils/status.hpp"

struct IntegerCase {
    const char* text;
    cpu_word_size expected;
};

class IntegerParserTest : public ::testing::TestWithParam<IntegerCase> {};

TEST_P(IntegerParserTest, ParsesIntegers) {
    auto param = GetParam();
    std::istringstream ss(param.text);
    EXPECT_EQ(TokenParser::integer(ss), param.expected);
}

INSTANTIATE_TEST_SUITE_P(
    IntegerCases,
    IntegerParserTest,
    ::testing::Values(
        IntegerCase{"0", 0},
        IntegerCase{"1", 1},
        IntegerCase{"7", 7},
        IntegerCase{"42", 42},
        IntegerCase{"128", 128}
    )
);

struct LetterCase {
    const char* text;
    uchar expected;
};

class LetterParserTest : public ::testing::TestWithParam<LetterCase> {};

TEST_P(LetterParserTest, ParsesRegisterLetters) {
    auto param = GetParam();
    std::istringstream ss(param.text);
    EXPECT_EQ(TokenParser::letter_idx(ss), param.expected);
}

INSTANTIATE_TEST_SUITE_P(
    LetterCases,
    LetterParserTest,
    ::testing::Values(
        LetterCase{"A", 0},
        LetterCase{"B", 1},
        LetterCase{"C", 2},
        LetterCase{"D", 3},
        LetterCase{"Z", 25}
    )
);

struct DirectionCase {
    const char* text;
    schar expected_dx;
    schar expected_dy;
};

class DirectionParserTest : public ::testing::TestWithParam<DirectionCase> {};

TEST_P(DirectionParserTest, ParsesDirections) {
    auto param = GetParam();
    std::istringstream ss(param.text);
    Status status;
    schar dx = 0;
    schar dy = 0;

    TokenParser::direction(ss, dx, dy, status);

    EXPECT_FALSE(status.p_err);
    EXPECT_EQ(dx, param.expected_dx);
    EXPECT_EQ(dy, param.expected_dy);
}

INSTANTIATE_TEST_SUITE_P(
    DirectionCases,
    DirectionParserTest,
    ::testing::Values(
        DirectionCase{"UP", 0, -1},
        DirectionCase{"DOWN", 0, 1},
        DirectionCase{"LEFT", -1, 0},
        DirectionCase{"RIGHT", 1, 0},
        DirectionCase{"UP", 0, -1}
    )
);
