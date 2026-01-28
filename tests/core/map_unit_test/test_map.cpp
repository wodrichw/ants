#include <gtest/gtest.h>

#include "map/map.hpp"
#include "map/world.hpp"
#include "app/globals.hpp"

TEST(MapChunkTest, ChunkConstructorCreatesTiles) {
    Chunk chunk(0, 0, true);
    EXPECT_EQ(chunk.tiles.size(), static_cast<size_t>(globals::CHUNK_AREA));
}

TEST(MapChunkTest, ChunkProtoRoundTripPreservesWalls) {
    Chunk chunk(0, 0, true);
    chunk[0].is_wall = false;

    ant_proto::Chunk msg = chunk.get_proto();
    Chunk restored(msg);

    EXPECT_FALSE(restored[0].is_wall);
}

TEST(MapChunkTest, ChunksAlignsToChunkLength) {
    Chunks chunks;
    long aligned = chunks.align(13);
    EXPECT_EQ(aligned % globals::CHUNK_LENGTH, 0);
}

TEST(MapChunkTest, ChunkMarkersGeneratedForRect) {
    Chunks chunks;
    Rect rect(0, 0, globals::CHUNK_LENGTH * 2, globals::CHUNK_LENGTH * 2);
    auto markers = chunks.get_chunk_markers(rect);
    EXPECT_FALSE(markers.empty());
}

struct ChunkIdCase {
    long x;
    long y;
    ulong expected;
};

class ChunkIdTest : public ::testing::TestWithParam<ChunkIdCase> {};

TEST_P(ChunkIdTest, ChunkIdMatchesExpected) {
    Chunks chunks;
    auto param = GetParam();
    EXPECT_EQ(chunks.get_chunk_id(param.x, param.y), param.expected);
}

INSTANTIATE_TEST_SUITE_P(
    ChunkIdCases,
    ChunkIdTest,
    ::testing::Values(
        ChunkIdCase{0, 0, 0},
        ChunkIdCase{globals::CHUNK_LENGTH, 0, 1},
        ChunkIdCase{0, globals::CHUNK_LENGTH, 3},
        ChunkIdCase{-globals::CHUNK_LENGTH, 0, 5},
        ChunkIdCase{0, -globals::CHUNK_LENGTH, 7}
    )
);

TEST(MapTest, DigClearsWallTiles) {
    Map map(true, [](long, long) {});
    map.dig(0, 0, 0, 0);
    EXPECT_FALSE(map.is_wall(0, 0));
}

TEST(MapTest, ExploreMarksTileInFov) {
    Map map(true, [](long, long) {});
    map.explore(0, 0);
    EXPECT_TRUE(map.is_explored(0, 0));
    EXPECT_TRUE(map.in_fov(0, 0));
}

TEST(MapTest, ResetTileClearsFov) {
    Map map(true, [](long, long) {});
    map.explore(0, 0);
    map.reset_tile(0, 0);
    EXPECT_FALSE(map.in_fov(0, 0));
}

TEST(MapTest, ResetFovClearsAllTiles) {
    Map map(true, [](long, long) {});
    map.explore(0, 0);
    map.explore(1, 1);
    map.reset_fov();
    EXPECT_FALSE(map.in_fov(0, 0));
    EXPECT_FALSE(map.in_fov(1, 1));
}

TEST(MapTest, GetChunkMarkersFromMapUsesChunks) {
    Map map(true, [](long, long) {});
    Rect rect(0, 0, globals::CHUNK_LENGTH, globals::CHUNK_LENGTH);
    auto markers = map.get_chunk_markers(rect);
    EXPECT_FALSE(markers.empty());
}

TEST(MapTest, CanPlaceRejectsWalls) {
    Map map(true, [](long, long) {});
    EXPECT_FALSE(map.can_place(0, 0));
}
