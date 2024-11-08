#pragma once

#include <libtcod/color.hpp>

#include "entity/rect.hpp"
#include "proto/entity.pb.h"
#include "ui/colors.hpp"

enum BuildingType { NURSERY };

struct Building {
    Rect const border = {};
    long id = 0;
    tcod::ColorRGB color = {};

    Building(Rect const& border, int id, tcod::ColorRGB color);
    Building(const ant_proto::Building& msg);
    virtual ~Building(){}
    virtual BuildingType get_type() const = 0;
    ant_proto::Building get_proto() const;
};

class Nursery : public Building {
   public:
    Nursery(long x, long y, int id)
        : Building(Rect::from_top_left(x, y, 3, 3), id, color::blue) {}

    Nursery(const ant_proto::Building& msg) : Building(msg) {}
    BuildingType get_type() const { return NURSERY; }
};

