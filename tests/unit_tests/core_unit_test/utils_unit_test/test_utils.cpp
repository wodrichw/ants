#include <gtest/gtest.h>

#include <filesystem>
#include <chrono>
#include <atomic>

#include "utils/math.hpp"
#include "utils/string.hpp"
#include "utils/status.hpp"
#include "utils/serializer.hpp"
#include "utils/thread_pool.hpp"
#include "proto/utils.pb.h"

struct DivCase {
    long a;
    long b;
    long expected;
};

class DivFloorTest : public ::testing::TestWithParam<DivCase> {};

TEST_P(DivFloorTest, DivFloorMatchesExpected) {
    auto param = GetParam();
    EXPECT_EQ(div_floor(param.a, param.b), param.expected);
}

INSTANTIATE_TEST_SUITE_P(
    DivFloorCases,
    DivFloorTest,
    ::testing::Values(
        DivCase{5, 2, 2},
        DivCase{4, 2, 2},
        DivCase{3, 2, 1},
        DivCase{-3, 2, -2},
        DivCase{3, -2, -2},
        DivCase{-3, -2, 1},
        DivCase{0, 1, 0},
        DivCase{7, 3, 2},
        DivCase{-7, 3, -3}
    )
);

struct TrimCase {
    const char* input;
    const char* expected;
    long expected_size;
};

class TrimCopyTest : public ::testing::TestWithParam<TrimCase> {};

TEST_P(TrimCopyTest, TrimCopyRemovesWhitespace) {
    auto param = GetParam();
    std::string input = param.input;
    auto trimmed = trim_copy(input);

    EXPECT_EQ(trimmed, param.expected);
    EXPECT_EQ(static_cast<long>(trimmed.size()), param.expected_size);
}

INSTANTIATE_TEST_SUITE_P(
    TrimCopyCases,
    TrimCopyTest,
    ::testing::Values(
        TrimCase{"  abc  ", "abc", 3},
        TrimCase{"\tfoo\n", "foo", 3},
        TrimCase{"   ", "", 0},
        TrimCase{"a b", "a b", 3},
        TrimCase{"", "", 0}
    )
);

struct EmptyCase {
    const char* input;
    bool expected;
};

class TrimEmptyTest : public ::testing::TestWithParam<EmptyCase> {};

TEST_P(TrimEmptyTest, TrimmedEmptyMatchesExpected) {
    auto param = GetParam();
    std::string trimmed = trim_copy(param.input);
    bool is_empty = trimmed.empty();
    EXPECT_EQ(is_empty, param.expected);
}

INSTANTIATE_TEST_SUITE_P(
    TrimEmptyCases,
    TrimEmptyTest,
    ::testing::Values(
        EmptyCase{" ", true},
        EmptyCase{"", true},
        EmptyCase{"\t", true},
        EmptyCase{"a", false},
        EmptyCase{"  a ", false}
    )
);

TEST(UtilsStatusTest, DefaultStatusIsOk) {
    Status status;
    EXPECT_FALSE(status.p_err);
}

TEST(UtilsStatusTest, ErrorSetsMessageAndFlag) {
    Status status;
    status.error("boom");
    EXPECT_TRUE(status.p_err);
    EXPECT_EQ(status.err_msg, "boom");
}

static std::string make_temp_path(const std::string& name) {
    auto ts = std::chrono::steady_clock::now().time_since_epoch().count();
    return (std::filesystem::temp_directory_path() /
            (name + std::to_string(ts)))
        .string();
}

TEST(UtilsSerializerTest, PacksAndUnpacksInteger) {
    std::string path = make_temp_path("packer_int_");
    ant_proto::Integer msg;
    msg.set_value(42);

    {
        Packer packer(path);
        ASSERT_TRUE(static_cast<bool>(packer));
        packer << msg;
    }

    ant_proto::Integer restored;
    {
        Unpacker unpacker(path);
        ASSERT_TRUE(unpacker.is_valid());
        unpacker >> restored;
    }

    EXPECT_EQ(restored.value(), 42);
    std::filesystem::remove(path);
}

TEST(UtilsSerializerTest, PacksAndUnpacksColor) {
    std::string path = make_temp_path("packer_color_");
    tcod::ColorRGB color{12, 34, 56};

    {
        Packer packer(path);
        ASSERT_TRUE(static_cast<bool>(packer));
        packer << color;
    }

    tcod::ColorRGB restored{};
    {
        Unpacker unpacker(path);
        ASSERT_TRUE(unpacker.is_valid());
        unpacker >> restored;
    }

    EXPECT_EQ(restored.r, 12);
    EXPECT_EQ(restored.g, 34);
    EXPECT_EQ(restored.b, 56);
    std::filesystem::remove(path);
}

struct SimpleJob {
    std::atomic<int>* counter = nullptr;
    void run() { counter->fetch_add(1); }
};

TEST(UtilsThreadPoolTest, SubmitsAndRunsJob) {
    std::atomic<int> count{0};
    ThreadPool<SimpleJob> pool(1);
    SimpleJob job{&count};
    pool.submit_job(job);
    pool.await_jobs();

    EXPECT_EQ(count.load(), 1);
}

TEST(UtilsThreadPoolTest, SubmitsMultipleJobs) {
    std::atomic<int> count{0};
    ThreadPool<SimpleJob> pool(1);
    SimpleJob job_a{&count};
    SimpleJob job_b{&count};
    pool.submit_job(job_a);
    pool.await_jobs();

    pool.submit_job(job_b);
    pool.await_jobs();

    EXPECT_EQ(count.load(), 2);
}
