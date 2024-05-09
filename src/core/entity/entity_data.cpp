#include "entity/entity_data.hpp"
#include "spdlog/spdlog.h"
#include "proto/entity.pb.h"
#include "utils/serializer.hpp"

EntityData::EntityData(char ch, long fov_radius, tcod::ColorRGB col)
    : x(0), y(0), ch(ch), fov_radius(fov_radius), col(col), last_rendered_pos() {}

EntityData::EntityData(long x, long y, char ch, long fov_radius, tcod::ColorRGB col)
    : x(x), y(y), ch(ch), fov_radius(fov_radius), col(col), last_rendered_pos() {
        SPDLOG_DEBUG("EntityData created at ({}, {})", x, y);
        SPDLOG_TRACE("EntityData FOV radius: {} char: {}", fov_radius, ch);
}

EntityData::EntityData(Unpacker& p) {
    ant_proto::EntityData msg;
    p >> msg;

    x = msg.x();
    y = msg.y();
    ch = msg.ch();
    fov_radius = msg.fov_radius();
}

Packer& operator<<(Packer& p, EntityData const& obj) {
    ant_proto::EntityData msg;
    msg.set_x(obj.x);
    msg.set_y(obj.y);
    msg.set_ch(obj.ch);
    msg.set_fov_radius(obj.fov_radius);
    return p << msg;
}