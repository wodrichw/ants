#ifndef __BUILDING_HPP
#define __BUILDING_HPP

#include <libtcod/color.hpp>

#include "ui/colors.hpp"
#include "spdlog/spdlog.h"

struct Building {
    const long x, y, w, h, id;  // bottom left (x, y), width, height
    const tcod::ColorRGB color;

    Building(long x, long y, int w, int h, int id, tcod::ColorRGB color)
        : x(x), y(y), w(w), h(h), id(id), color(color) {}
};

struct Nursery {
    Building building;

    Nursery(long x, long y, int id)
        : building(x, y, 3, 3, id, color::blue) {}
};

#endif  // __BUILDING_HPP
