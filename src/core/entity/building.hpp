#pragma once

#include <libtcod/color.hpp>

#include "ui/colors.hpp"
#include "entity/rect.hpp"

struct Building {
    Rect const border;
    const long id;
    const tcod::ColorRGB color;

    Building(Rect const& border, int id, tcod::ColorRGB color) : border(border), id(id), color(color) {}
};

class Nursery : public Building {
   public:
    Nursery(long x, long y, int id)
        : Building(Rect::from_top_left(x, y, 3, 3), id, color::blue) {}
};
