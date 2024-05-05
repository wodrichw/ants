#include "entity/entity_data.hpp"
#include "spdlog/spdlog.h"

EntityData::EntityData(char ch, long fov_radius, tcod::ColorRGB col)
    : x(0), y(0), ch(ch), fov_radius(fov_radius), col(col), last_rendered_pos() {}

EntityData::EntityData(long x, long y, char ch, long fov_radius, tcod::ColorRGB col)
    : x(x), y(y), ch(ch), fov_radius(fov_radius), col(col), last_rendered_pos() {
        SPDLOG_DEBUG("EntityData created at ({}, {})", x, y);
        SPDLOG_TRACE("EntityData FOV radius: {} char: {}", fov_radius, ch);
}

Packer& operator<<(Packer& p, EntityData const& obj) {
    p << obj.x << obj.y << obj.ch << obj.fov_radius;
    return p << obj.col.r << obj.col.g << obj.col.b;
}

Unpacker& operator>>(Unpacker& p, EntityData& obj) {
    p >> obj.x >> obj.y >> obj.ch >> obj.fov_radius;
    return p >> obj.col.r >> obj.col.g >> obj.col.b;
}