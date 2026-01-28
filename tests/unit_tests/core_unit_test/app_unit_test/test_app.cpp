#include <gtest/gtest.h>

#include "app/arg_parse.hpp"

TEST(AppArgumentParserTest, ParsesStringArgument) {
    const char* argv[] = {"ants", "--map_path", "level.txt"};
    ArgumentParser parser(3, const_cast<char**>(argv));

    EXPECT_TRUE(parser.hasKey("map_path"));
    EXPECT_EQ(parser.getString("map_path"), "level.txt");
}

TEST(AppArgumentParserTest, ParsesNumericArguments) {
    const char* argv[] = {"ants", "--value", "10", "--ratio", "2.5"};
    ArgumentParser parser(5, const_cast<char**>(argv));

    EXPECT_EQ(parser.getInt("value"), 10);
    EXPECT_DOUBLE_EQ(parser.getDouble("ratio"), 2.5);
}

TEST(AppArgumentParserTest, ParsesBoolArguments) {
    const char* argv[] = {"ants", "--no_render"};
    ArgumentParser parser(2, const_cast<char**>(argv));

    EXPECT_TRUE(parser.getBool("no_render"));
}

TEST(AppProjectArgumentsTest, ConstructorSetsFlags) {
    ProjectArguments args("map.txt", "save.dat", "rec.dat", "play.dat", false, true, false);

    EXPECT_FALSE(args.is_render);
    EXPECT_TRUE(args.is_debug_graphics);
    EXPECT_FALSE(args.is_walls_enabled);
}

TEST(AppProjectArgumentsTest, ParsesCliFlags) {
    const char* argv[] = {"ants", "--no_render", "--disable_walls"};
    ProjectArguments args(3, const_cast<char**>(argv));

    EXPECT_FALSE(args.is_render);
    EXPECT_FALSE(args.is_walls_enabled);
}
