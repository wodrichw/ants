#include <gtest/gtest.h>

#include "entity/rect.hpp"
#include "entity/inventory.hpp"
#include "entity/entity_data.hpp"
#include "entity/ant.hpp"
#include "ui/colors.hpp"
#include "utils/thread_pool.hpp"

struct RectInsideCase {
    long x;
    long y;
    bool expected;
};

class RectInsideTest : public ::testing::TestWithParam<RectInsideCase> {};

TEST_P(RectInsideTest, IsInsideMatchesExpected) {
    Rect rect(0, 0, 4, 4);
    auto param = GetParam();
    EXPECT_EQ(rect.is_inside(param.x, param.y), param.expected);
}

INSTANTIATE_TEST_SUITE_P(
    RectInsideCases,
    RectInsideTest,
    ::testing::Values(
        RectInsideCase{0, 0, true},
        RectInsideCase{3, 3, true},
        RectInsideCase{4, 4, false},
        RectInsideCase{-1, 0, false},
        RectInsideCase{0, -1, false},
        RectInsideCase{2, 1, true},
        RectInsideCase{3, 0, true},
        RectInsideCase{0, 3, true}
    )
);

TEST(EntityInventoryTest, ItemInfoMapHasDefaultItems) {
    ItemInfoMap map;
    EXPECT_EQ(map.size(), 3u);
}

TEST(EntityInventoryTest, InventoryStartsEmpty) {
    ItemInfoMap map;
    Inventory inv(2, 5, 100, map);
    EXPECT_FALSE(inv.has(DIRT));
    EXPECT_EQ(inv.size(), 3u);
}

TEST(EntityInventoryTest, InventoryAddMarksHasItem) {
    ItemInfoMap map;
    Inventory inv(2, 5, 100, map);
    ulong delta = 2;
    inv.add(DIRT, delta);
    EXPECT_TRUE(inv.has(DIRT));
}

TEST(EntityInventoryTest, InventoryRespectsWeightLimit) {
    ItemInfoMap map;
    Inventory inv(1, 1, 10, map);  // dirt weighs 20
    EXPECT_EQ(inv.max_space_for_item(DIRT), 0u);
}

TEST(EntityInventoryTest, InventoryRemoveDecreasesCount) {
    ItemInfoMap map;
    Inventory inv(2, 5, 100, map);
    ulong add = 2;
    inv.add(DIRT, add);
    ulong remove = 1;
    inv.remove(DIRT, remove);
    EXPECT_TRUE(inv.has(DIRT));
}

TEST(EntityInventoryTest, InventoryProtoRoundTrip) {
    ItemInfoMap map;
    Inventory inv(2, 5, 100, map);
    ulong add = 3;
    inv.add(FOOD, add);

    ant_proto::Inventory msg = inv.get_proto();
    Inventory restored(msg, map);
    EXPECT_TRUE(restored.has(FOOD));
}

TEST(EntityDataTest, ConstructorSetsValues) {
    tcod::ColorRGB col{1, 2, 3};
    EntityData data(4, 5, '@', 6, col);
    EXPECT_EQ(data.x, 4);
    EXPECT_EQ(data.y, 5);
    EXPECT_EQ(data.ch, '@');
    EXPECT_EQ(data.fov_radius, 6);
}

TEST(EntityDataTest, ProtoRoundTrip) {
    tcod::ColorRGB col{7, 8, 9};
    EntityData data(1, 2, 'a', 3, col);

    auto msg = data.get_proto();
    EntityData restored(msg);

    EXPECT_EQ(restored.x, 1);
    EXPECT_EQ(restored.y, 2);
    EXPECT_EQ(restored.ch, 'a');
    EXPECT_EQ(restored.fov_radius, 3);
}

TEST(EntityDataTest, DefaultRenderPosition) {
    EntityData data('*', 1, color::white);
    EXPECT_FALSE(data.last_rendered_pos.requires_update);
}

TEST(EntityAntTest, PlayerTypeIsPlayer) {
    ItemInfoMap map;
    EntityData data('*', 1, color::white);
    Player player(data, map);
    EXPECT_EQ(player.get_type(), PLAYER);
}

TEST(EntityAntTest, WorkerTypeIsWorker) {
    ItemInfoMap map;
    ulong clock = 0;
    ThreadPool<AsyncProgramJob> pool(1);
    EntityData data('*', 1, color::white);
    Worker worker(data, clock, map, pool);
    EXPECT_EQ(worker.get_type(), WORKER);
}

TEST(EntityAntTest, ClickCallbackTogglesColor) {
    ItemInfoMap map;
    EntityData data('*', 1, color::light_green);
    Player player(data, map);

    player.click_callback(0, 0);
    EXPECT_EQ(player.data.col, color::dark_yellow);
}
