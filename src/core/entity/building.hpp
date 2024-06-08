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
        SPDLOG_DEBUG("Unpacked building - id: {}", id);
    }

    virtual ~Building(){}

    virtual BuildingType get_type() const = 0;

    friend Packer& operator<<(Packer& p, Building const& obj) {
        SPDLOG_DEBUG("Packing building - id: {} x: {} y: {}", obj.id, obj.border.x1, obj.border.y1);
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
    BuildingType get_type() const { return NURSERY; }
    friend Packer& operator<<(Packer& p, Nursery const& obj) {
        SPDLOG_DEBUG("Packing nursery");
        return p << static_cast<Building const&>(obj);
    }
};
