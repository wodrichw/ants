#include <gtest/gtest.h>

#include <string>

#include "hardware/label_map.hpp"

struct LabelCase {
    ushort address;
    const char* label;
};

class LabelMapTest : public ::testing::TestWithParam<LabelCase> {};

TEST_P(LabelMapTest, InsertAndLookup) {
    LabelMap map;
    auto param = GetParam();
    map.insert(param.address, param.label);

    EXPECT_EQ(map.at(param.address), param.label);
    EXPECT_EQ(map.at(std::string(param.label)), param.address);
}

INSTANTIATE_TEST_SUITE_P(
    LabelCases,
    LabelMapTest,
    ::testing::Values(
        LabelCase{0, "L0"},
        LabelCase{1, "L1"},
        LabelCase{2, "L2"},
        LabelCase{3, "L3"},
        LabelCase{4, "L4"},
        LabelCase{5, "L5"},
        LabelCase{6, "L6"},
        LabelCase{7, "L7"},
        LabelCase{8, "L8"},
        LabelCase{9, "L9"},
        LabelCase{10, "LA"},
        LabelCase{11, "LB"},
        LabelCase{12, "LC"},
        LabelCase{13, "LD"},
        LabelCase{14, "LE"},
        LabelCase{15, "LF"},
        LabelCase{16, "LG"},
        LabelCase{17, "LH"},
        LabelCase{18, "LI"},
        LabelCase{19, "LJ"}
    )
);
