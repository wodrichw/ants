#include <gtest/gtest.h>

#include "hardware/software_manager.hpp"
#include "hardware/command_config.hpp"

TEST(SoftwareManagerTest, StartsWithNoCode) {
    CommandMap map;
    SoftwareManager manager(map);
    EXPECT_FALSE(manager.has_code());
}

TEST(SoftwareManagerTest, AddLinesLoadsProgram) {
    CommandMap map;
    SoftwareManager manager(map);

    manager.add_lines({"NOP"});

    EXPECT_TRUE(manager.has_code());
    EXPECT_GT(manager.get().size(), 0u);
}

TEST(SoftwareManagerTest, AddLinesInvalidClearsProgram) {
    CommandMap map;
    SoftwareManager manager(map);

    manager.add_lines({"BAD"});

    EXPECT_FALSE(manager.has_code());
    EXPECT_TRUE(manager.get().is_empty());
}

TEST(SoftwareManagerTest, GetLinesAppendsEmptyLine) {
    CommandMap map;
    SoftwareManager manager(map);

    manager.add_lines({"NOP"});

    std::vector<std::string> lines;
    manager.get_lines(lines);

    ASSERT_FALSE(lines.empty());
    EXPECT_EQ(lines.back(), "");
}

TEST(SoftwareManagerTest, AssignStoresCurrentCode) {
    CommandMap map;
    SoftwareManager manager(map);

    manager.add_lines({"NOP"});
    manager.assign(1);
    manager.add_lines({"MOVE"});

    EXPECT_FALSE(manager[1].is_empty());
}

TEST(SoftwareManagerTest, OperatorIndexFallsBackToCurrent) {
    CommandMap map;
    SoftwareManager manager(map);

    EXPECT_TRUE(manager[42].is_empty());
}

TEST(SoftwareManagerTest, GetProtoReflectsAssignments) {
    CommandMap map;
    SoftwareManager manager(map);

    manager.add_lines({"NOP"});
    manager.assign(2);
    manager.add_lines({"MOVE"});

    auto proto = manager.get_proto();
    EXPECT_GE(proto.ant_machine_codes_size(), 1);
    EXPECT_GE(proto.ant_code_records_size(), 1);
}

TEST(SoftwareManagerTest, AddLinesAfterAssignCreatesNewCurrent) {
    CommandMap map;
    SoftwareManager manager(map);

    manager.add_lines({"NOP"});
    manager.assign(3);
    manager.add_lines({"MOVE"});

    EXPECT_FALSE(manager.get().is_empty());
}

TEST(SoftwareManagerTest, GetLinesClearsOnDeparseError) {
    CommandMap map;
    SoftwareManager manager(map);

    std::vector<std::string> lines;
    manager.get_lines(lines);

    EXPECT_FALSE(lines.empty());
}

TEST(SoftwareManagerTest, AssignDoesNotClearCurrentImmediately) {
    CommandMap map;
    SoftwareManager manager(map);

    manager.add_lines({"NOP"});
    manager.assign(4);

    EXPECT_TRUE(manager.has_code());
}
