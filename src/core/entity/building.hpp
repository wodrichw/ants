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

    Building(Rect const& border, int id, tcod::ColorRGB color)
        : border(border), id(id), color(color) {}

    Building(const ant_proto::Building& msg) :
        border(msg.rect()),
        id(msg.id()),
        color(tcod::ColorRGB{
                static_cast<uint8_t>(msg.col().r()),
                static_cast<uint8_t>(msg.col().g()),
                static_cast<uint8_t>(msg.col().b())
            })
    {
        SPDLOG_DEBUG("Unpacked building - id: {}", id);
    }

    virtual ~Building(){}

    virtual BuildingType get_type() const = 0;

    ant_proto::Building get_proto() const {
        ant_proto::Building msg;
        msg.set_id(id);
        *msg.mutable_rect() = border.get_proto();
        ant_proto::Color& col_msg = *msg.mutable_col();
        col_msg.set_r(color.r);
        col_msg.set_g(color.g);
        col_msg.set_b(color.b);
        return msg;
    }
};

class Nursery : public Building {
   public:
    Nursery(long x, long y, int id)
        : Building(Rect::from_top_left(x, y, 3, 3), id, color::blue) {}

    Nursery(const ant_proto::Building& msg) : Building(msg) {}
    BuildingType get_type() const { return NURSERY; }
};

