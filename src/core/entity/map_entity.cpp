#include "entity/map_entity.hpp"
#include "spdlog/spdlog.h"

MapData::MapData(long x, long y, char ch, int fov_radius, tcod::ColorRGB col)
    : x(x), y(y), ch(ch), fov_radius(fov_radius), col(col), last_rendered_pos() {
        SPDLOG_DEBUG("MapData created at ({}, {})", x, y);
        SPDLOG_TRACE("MapData FOV radius: {} char: {}", fov_radius, ch);
}