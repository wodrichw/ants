#include <gtest/gtest.h>

#include "proto/utils.pb.h"
#include "proto/entity.pb.h"
#include "proto/replay.pb.h"
#include "proto/hardware.pb.h"

TEST(ProtoTest, IntegerSerializesRoundTrip) {
    ant_proto::Integer msg;
    msg.set_value(123);

    std::string data;
    ASSERT_TRUE(msg.SerializeToString(&data));

    ant_proto::Integer restored;
    ASSERT_TRUE(restored.ParseFromString(data));
    EXPECT_EQ(restored.value(), 123);
}

TEST(ProtoTest, RectFieldsSetAndGet) {
    ant_proto::Rect rect;
    rect.set_x1(1);
    rect.set_y1(2);
    rect.set_w(3);
    rect.set_h(4);

    EXPECT_EQ(rect.x1(), 1);
    EXPECT_EQ(rect.y1(), 2);
    EXPECT_EQ(rect.w(), 3);
    EXPECT_EQ(rect.h(), 4);
}

TEST(ProtoTest, EntityDataStoresValues) {
    ant_proto::EntityData data;
    data.set_x(5);
    data.set_y(6);
    data.set_ch('@');
    data.set_fov_radius(7);

    EXPECT_EQ(data.x(), 5);
    EXPECT_EQ(data.y(), 6);
    EXPECT_EQ(data.ch(), '@');
    EXPECT_EQ(data.fov_radius(), 7);
}

TEST(ProtoTest, ReplayHeaderStoresVersion) {
    ant_proto::ReplayHeader header;
    header.set_version(1);
    auto* env = header.mutable_environment();
    env->set_region_seed_x(2);
    env->set_region_seed_y(3);

    EXPECT_EQ(header.version(), 1u);
    EXPECT_EQ(header.environment().region_seed_x(), 2u);
    EXPECT_EQ(header.environment().region_seed_y(), 3u);
}

TEST(ProtoTest, MachineCodeStoresBytesAndLabels) {
    ant_proto::MachineCode code;
    code.set_code("\x01\x02");
    auto* label = code.add_labels();
    label->set_address(1);
    label->set_label("L1");

    EXPECT_EQ(code.code().size(), 2u);
    EXPECT_EQ(code.labels_size(), 1);
    EXPECT_EQ(code.labels(0).label(), "L1");
}
