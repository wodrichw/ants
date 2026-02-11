#include <gtest/gtest.h>

#include <vector>

#include "map/map.hpp"
#include "ui/colors.hpp"

class TestEntity : public MapEntity {
   public:
    EntityData data;
    bool requested_move = false;
    bool move_called = false;
    bool empty_called = false;
    bool full_called = false;
    Map* map = nullptr;

    TestEntity(long x, long y, Map* map)
        : data(x, y, '@', 1, color::white), map(map) {}

    EntityData& get_data() override { return data; }

    void move_callback(EntityMoveUpdate const&) override { move_called = true; }

    void click_callback(long, long) override {}

    void request_move() override {
        requested_move = true;
        if(map != nullptr) map->remove_entity(*this);
    }

    void handle_empty_space(uchar) override { empty_called = true; }

    void handle_full_space(uchar) override { full_called = true; }

    MapEntityType get_type() const override { return WORKER; }
};

struct DigCase {
    long x1;
    long y1;
    long x2;
    long y2;
    long check_x;
    long check_y;
};

class MapDigEdgeCaseTest : public ::testing::TestWithParam<DigCase> {};

TEST_P(MapDigEdgeCaseTest, DigHandlesReversedOrNegativeCoords) {
    Map map(true, [](long, long) {});
    auto param = GetParam();

    map.dig(param.x1, param.y1, param.x2, param.y2);

    EXPECT_FALSE(map.is_wall(param.check_x, param.check_y));
}

INSTANTIATE_TEST_SUITE_P(
    MapDigEdgeCases,
    MapDigEdgeCaseTest,
    ::testing::Values(
        DigCase{2, 2, 0, 0, 1, 1},
        DigCase{0, 0, 2, 2, 2, 2},
        DigCase{-2, -2, -4, -4, -3, -3},
        DigCase{-4, -2, -2, -4, -3, -3},
        DigCase{5, -1, 3, -3, 4, -2},
        DigCase{-1, 5, -3, 3, -2, 4},
        DigCase{1, -1, -1, 1, 0, 0},
        DigCase{10, 10, 8, 12, 9, 11},
        DigCase{-10, 10, -12, 8, -11, 9},
        DigCase{3, 7, 7, 3, 5, 5}
    )
);

struct MoveCase {
    long start_x;
    long start_y;
    long dx;
    long dy;
};

class MapMoveBlockedByWallTest : public ::testing::TestWithParam<MoveCase> {};

TEST_P(MapMoveBlockedByWallTest, MoveBlockedWhenWallsEnabled) {
    Map map(true, [](long, long) {});
    auto param = GetParam();

    TestEntity entity(param.start_x, param.start_y, &map);
    map.add_entity(entity);

    bool moved = map.move_entity(entity, param.dx, param.dy);

    EXPECT_FALSE(moved);
    EXPECT_EQ(entity.get_data().x, param.start_x);
    EXPECT_EQ(entity.get_data().y, param.start_y);
}

INSTANTIATE_TEST_SUITE_P(
    MapMoveBlockedByWallCases,
    MapMoveBlockedByWallTest,
    ::testing::Values(
        MoveCase{0, 0, 1, 0},
        MoveCase{0, 0, -1, 0},
        MoveCase{0, 0, 0, 1},
        MoveCase{0, 0, 0, -1},
        MoveCase{2, 2, 1, 0},
        MoveCase{-2, -2, -1, 0},
        MoveCase{5, -3, 0, 1},
        MoveCase{-4, 7, 1, 0},
        MoveCase{8, 8, -1, -1},
        MoveCase{-6, -6, 1, 1}
    )
);
