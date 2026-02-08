#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <string>

#include "google/protobuf/util/message_differencer.h"

#include "app/arg_parse.hpp"
#include "app/engine_state.hpp"
#include "engine.pb.h"
#include "map.pb.h"
#include "replay.pb.h"
#include "ui/render.hpp"
#include "utils/serializer.hpp"

#include "save_restore_seeds.h"

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

static std::string make_temp_path(const std::string& prefix,
                                  const SaveRestoreSeed& seed) {
    auto ts = std::chrono::steady_clock::now().time_since_epoch().count();
    return (std::filesystem::temp_directory_path() /
            (prefix + std::to_string(seed.seed_x) + "_" +
             std::to_string(seed.seed_y) + "_" + std::to_string(ts)))
        .string();
}

class SaveRestoreRoundTripTest
    : public ::testing::TestWithParam<SaveRestoreSeed> {};

TEST_P(SaveRestoreRoundTripTest, SavesAndRestoresEngineState) {
    const SaveRestoreSeed seed = GetParam();
    const bool walls_enabled = (seed.seed_x % 2u) == 0u;

    std::string save_path = make_temp_path("ants_save_restore_", seed);
    std::string roundtrip_path =
        make_temp_path("ants_save_restore_roundtrip_", seed);

    ProjectArguments config("", save_path, "", "", false, false,
                            walls_enabled);
    NoneRenderer renderer;

    ant_proto::ReplayEnvironment env;
    env.set_region_seed_x(seed.seed_x);
    env.set_region_seed_y(seed.seed_y);
    env.set_is_walls_enabled(walls_enabled);
    env.set_map_path("");

    EngineState original(config, &renderer, &env);
    original.action_move_player(1, 0);
    original.action_create_ant();
    original.action_add_program_lines({"LDI A 1", "INC A"});
    original.action_go_down();
    original.action_go_up();

    {
        Packer packer(save_path);
        ASSERT_TRUE(static_cast<bool>(packer));
        packer << original;
    }

    ASSERT_TRUE(std::filesystem::exists(save_path));

    ant_proto::EngineState saved_proto;
    {
        Unpacker unpacker(save_path);
        ASSERT_TRUE(unpacker.is_valid());
        unpacker >> saved_proto;
    }

    EngineState restored(saved_proto, config, &renderer);

    {
        Packer packer(roundtrip_path);
        ASSERT_TRUE(static_cast<bool>(packer));
        packer << restored;
    }

    ant_proto::EngineState roundtrip_proto;
    {
        Unpacker unpacker(roundtrip_path);
        ASSERT_TRUE(unpacker.is_valid());
        unpacker >> roundtrip_proto;
    }

    google::protobuf::util::MessageDifferencer differencer;
    differencer.set_repeated_field_comparison(
        google::protobuf::util::MessageDifferencer::AS_SET);
    const auto* chunk_desc = ant_proto::Chunk::descriptor();
    ASSERT_NE(chunk_desc, nullptr);
    const auto* update_parity_field =
        chunk_desc->FindFieldByName("update_parity");
    ASSERT_NE(update_parity_field, nullptr);
    differencer.IgnoreField(update_parity_field);
    EXPECT_TRUE(differencer.Compare(saved_proto, roundtrip_proto));

    std::filesystem::remove(save_path);
    std::filesystem::remove(roundtrip_path);
}

static std::string SaveRestoreSeedName(
    const ::testing::TestParamInfo<SaveRestoreSeed>& info) {
    return "Seed_" + std::to_string(info.param.seed_x) + "_" +
           std::to_string(info.param.seed_y);
}

INSTANTIATE_TEST_SUITE_P(SaveRestoreSeeds,
                         SaveRestoreRoundTripTest,
                         ::testing::ValuesIn(all_save_restore_seeds()),
                         SaveRestoreSeedName);
