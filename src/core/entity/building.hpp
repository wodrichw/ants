#pragma once

#include <libtcod/color.hpp>

#include "entity/rect.hpp"
#include "proto/entity.pb.h"
#include "ui/colors.hpp"
#include "utils/serializer.hpp"

enum BuildingType { NURSERY };

struct Building {
    Rect const border;
    long id;
    tcod::ColorRGB color;

    Building(Rect const& border, int id, tcod::ColorRGB color)
        : border(border), id(id), color(color) {}
    Building(Unpacker& p) : border(p) {
        ant_proto::Building msg;
        p >> msg >> color;

        id = msg.id();
    }

    friend Packer& operator<<(Packer& p, Building const& obj) {
        ant_proto::Building msg;
        msg.set_id(obj.id);
        return p << obj.border << msg << obj.color;
    }
};

class Nursery : public Building {
   public:
    Nursery(long x, long y, int id)
        : Building(Rect::from_top_left(x, y, 3, 3), id, color::blue) {}

    Nursery(Unpacker& p) : Building(p) {}
};
