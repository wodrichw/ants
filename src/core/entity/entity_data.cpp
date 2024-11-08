#include "entity/entity_data.hpp"
#include <libtcod/color.hpp>

#include "proto/entity.pb.h"
#include "spdlog/spdlog.h"

EntityData::EntityData(char ch, long fov_radius, tcod::ColorRGB col)
    : x(0),
      y(0),
      ch(ch),
      fov_radius(fov_radius),
      col(col),
      last_rendered_pos() {}

EntityData::EntityData(long x, long y, char ch, long fov_radius,
                       tcod::ColorRGB col)
    : x(x),
      y(y),
      ch(ch),
      fov_radius(fov_radius),
      col(col),
      last_rendered_pos() {
    SPDLOG_DEBUG("EntityData created at ({}, {})", x, y);
    SPDLOG_TRACE("EntityData FOV radius: {} char: {}", fov_radius, ch);
}

EntityData::EntityData(const ant_proto::EntityData& msg):
    x(msg.x()),
    y(msg.y()),
    ch(msg.ch()),
    fov_radius(msg.fov_radius()),
    col(tcod::ColorRGB{
            static_cast<uint8_t>(msg.color().r()),
            static_cast<uint8_t>(msg.color().g()),
            static_cast<uint8_t>(msg.color().b())
        })

{
    // SPDLOG_TRACE("Unpacked entity data - x: {} y: {} ch: {} fov_radius: {}, col {}", x, y, ch, fov_radius, col);
}


ant_proto::EntityData EntityData::get_proto() const {
    ant_proto::EntityData entity_data_msg;
    ant_proto::Color& col_msg = *entity_data_msg.mutable_color();

    col_msg.set_r(col.r);
    col_msg.set_g(col.g);
    col_msg.set_b(col.b);

    entity_data_msg.set_x(x);
    entity_data_msg.set_y(y);
    entity_data_msg.set_ch(ch);
    entity_data_msg.set_fov_radius(fov_radius);

    return entity_data_msg;
}

