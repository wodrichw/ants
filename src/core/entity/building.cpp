#include "entity/building.hpp"

#include "spdlog/spdlog.h"

Building::Building(Rect const& border, int id, tcod::ColorRGB color)
    : border(border), id(id), color(color) {}

Building::Building(const ant_proto::Building& msg)
    : border(msg.rect()),
    id(static_cast<long>(msg.id())),
      color(tcod::ColorRGB{static_cast<uint8_t>(msg.col().r()),
                           static_cast<uint8_t>(msg.col().g()),
                           static_cast<uint8_t>(msg.col().b())}) {
    SPDLOG_DEBUG("Unpacked building - id: {}", id);
}

ant_proto::Building Building::get_proto() const {
    ant_proto::Building msg;
    msg.set_id(id);
    *msg.mutable_rect() = border.get_proto();
    ant_proto::Color& col_msg = *msg.mutable_col();
    col_msg.set_r(color.r);
    col_msg.set_g(color.g);
    col_msg.set_b(color.b);
    return msg;
}
