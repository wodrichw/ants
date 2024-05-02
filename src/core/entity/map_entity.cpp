#include "entity/map_entity.hpp"
#include "spdlog/spdlog.h"

EntityData::EntityData(long x, long y, char ch, int fov_radius, tcod::ColorRGB col)
    : x(x), y(y), ch(ch), fov_radius(fov_radius), col(col), last_rendered_pos() {
        SPDLOG_DEBUG("EntityData created at ({}, {})", x, y);
        SPDLOG_TRACE("EntityData FOV radius: {} char: {}", fov_radius, ch);
}