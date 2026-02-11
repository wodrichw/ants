#include <gtest/gtest.h>

#include <string>

#include "hardware/machine_code.hpp"

struct MachineCodeCase {
    int code_size;
    int label_count;
};

class MachineCodeTest : public ::testing::TestWithParam<MachineCodeCase> {};

TEST_P(MachineCodeTest, RoundTripAndSize) {
    MachineCode code;
    auto param = GetParam();

    for (int i = 0; i < param.code_size; ++i) {
        code.code.push_back(static_cast<uchar>(i));
    }
    for (int i = 0; i < param.label_count; ++i) {
        code.labels.insert(static_cast<ushort>(i), "L" + std::to_string(i));
    }

    EXPECT_EQ(code.size(), static_cast<ulong>(param.code_size));
    EXPECT_EQ(code.is_empty(), param.code_size == 0 && param.label_count == 0);

    auto proto = code.get_proto();
    EXPECT_EQ(proto.code().size(), param.code_size);
    EXPECT_EQ(proto.labels_size(), param.label_count);

    MachineCode restored(proto);
    EXPECT_EQ(restored.size(), static_cast<ulong>(param.code_size));
    EXPECT_EQ(restored.labels.size(), static_cast<ulong>(param.label_count));
}

INSTANTIATE_TEST_SUITE_P(
    MachineCodeCases,
    MachineCodeTest,
    ::testing::Values(
        MachineCodeCase{0, 0},
        MachineCodeCase{1, 0},
        MachineCodeCase{2, 1},
        MachineCodeCase{3, 2},
        MachineCodeCase{4, 0},
        MachineCodeCase{5, 3},
        MachineCodeCase{6, 1},
        MachineCodeCase{7, 2},
        MachineCodeCase{8, 4},
        MachineCodeCase{9, 5}
    )
);
