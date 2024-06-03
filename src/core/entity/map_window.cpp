#include "entity/map_window.hpp"

#include <libtcod.hpp>

#include "proto/entity.pb.h"
#include "spdlog/spdlog.h"

using ulong = unsigned long;

MapWindow::MapWindow(Rect const& border)
    : border(border),
      rooms(),
      corridors(),
      map(new TCODMap(border.w, border.h)) {
    SPDLOG_INFO("Creating map of size {}x{}", border.w, border.h);
}

MapWindow::MapWindow(Unpacker& p) : border(p) {
    ant_proto::MapWindow msg;
    p >> msg;

    ulong room_count = msg.room_count();
    ulong corridor_count = msg.corridor_count();
    SPDLOG_DEBUG("Unpacking map window - room count: {} - corridor count: {}", room_count, corridor_count);
    rooms.reserve(room_count);
    for(ulong i = 0; i < room_count; ++i) {
        rooms.emplace_back(p);
    }

    corridors.reserve(corridor_count);
    for(ulong i = 0; i < corridor_count; ++i) {
        corridors.emplace_back(p);
    }
    SPDLOG_TRACE("Completed unpacking map window");
}

MapWindow::~MapWindow() {
    SPDLOG_INFO("Destroying map");
    delete map;
    SPDLOG_DEBUG("Map destroyed");
}

void MapWindow::set_center(long x, long y) {
    SPDLOG_TRACE("Setting map window center to ({}, {})", x, y);
    border.set_center(x, y);
}

bool MapWindow::in_fov(long x, long y) const {
    long local_x, local_y;
    bool is_valid;
    to_local_coords(x, y, local_x, local_y, is_valid);
    // SPDLOG_TRACE("Checking if ({}, {}) is in FOV", local_x, local_y);
    return is_valid && map->isInFov(local_x, local_y);
}

void MapWindow::compute_fov(long x, long y, long radius) {
    long local_x, local_y;
    bool is_valid;
    to_local_coords(x, y, local_x, local_y, is_valid);
    if(!is_valid) return;

    SPDLOG_TRACE("Computing FOV from ({}, {}) with radius {}", local_x, local_y,
                 radius);
    map->computeFov(local_x, local_y, radius);
    SPDLOG_TRACE("FOV computed");
}

void MapWindow::set_wall(long x, long y) {
    long local_x, local_y;
    bool is_valid;
    to_local_coords(x, y, local_x, local_y, is_valid);
    if(!is_valid) {
        // SPDLOG_ERROR("Invalid coordinates ({}, {}) when set wall", x, y);
        return;
    }

    // SPDLOG_TRACE("Setting wall at ({}, {})", local_x, local_y);
    map->setProperties(local_x, local_y, false, false);
}

void MapWindow::set_floor(long x, long y) {
    long local_x, local_y;
    bool is_valid;
    to_local_coords(x, y, local_x, local_y, is_valid);
    if(!is_valid) {
        // SPDLOG_ERROR("Invalid coordinates ({}, {}) when set floor", x, y);
        return;
    }

    // SPDLOG_TRACE("Setting floor at ({}, {})", local_x, local_y);
    map->setProperties(local_x, local_y, true, true);
}

void MapWindow::to_local_coords(long x, long y, long& local_x, long& local_y,
                                bool& is_valid) const {
    local_x = x - border.x1;
    local_y = y - border.y1;
    is_valid = local_x >= 0 && local_x < border.w && local_y >= 0 &&
               local_y < border.h;
}

Packer& operator<<(Packer& p, MapWindow const& obj) {
    SPDLOG_DEBUG("Packing map window - room count: {} - corridor count: {}", obj.rooms.size(), obj.corridors.size());
    ant_proto::MapWindow msg;
    msg.set_room_count(obj.rooms.size());
    msg.set_corridor_count(obj.corridors.size());

    p << obj.border << msg;
    for(Rect const& room : obj.rooms) {
        p << room;
    }
    for(Rect const& corridor : obj.corridors) {
        p << corridor;
    }
    SPDLOG_TRACE("Completed packing map window");
    return p;
}
