#ifndef __BUILDING_HPP
#define __BUILDING_HPP

#include <libtcod/color.hpp>

#include "ui/colors.hpp"
#include "entity/map.hpp"
#include "spdlog/spdlog.h"

class Building {
   protected:
   public:
    Building(long x, long y, int w, int h, int id, tcod::ColorRGB color,
             Map& map)
        : x(x), y(y), w(w), h(h), id(id), color(color) {
        SPDLOG_TRACE("Building created at ({}, {})", x, y);
        SPDLOG_TRACE("Building width: {}, height: {}", w, h);
        // Mark building location in map
        for(long xi = x; xi < x + w; ++xi) {
            for(long yi = y; yi < y + h; ++yi) {
                map.getTile(xi, yi).bldgId.emplace(id);
            }
        }
        SPDLOG_TRACE("Building initialized and marked on map");
    }
    const long x, y, w, h, id;  // bottom left (x, y), width, height
    const tcod::ColorRGB color;
};

class Nursery : public Building {
   public:
    Nursery(long x, long y, int id, Map& map)
        : Building(x, y, 3, 3, id, color::blue, map) {
            SPDLOG_DEBUG("Nursery created at ({}, {})", x, y);
        }
};

#endif  // __BUILDING_HPP
