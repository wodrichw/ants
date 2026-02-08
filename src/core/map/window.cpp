#include "map/window.hpp"

#include <libtcod.hpp>

#include "app/globals.hpp"
#include "spdlog/spdlog.h"

MapWindow::MapWindow(Rect const& border)
    : border(border),
      rooms(),
      corridors(),
      map(new TCODMap(border.w, border.h)) {
    SPDLOG_INFO("Creating map of size {}x{}", border.w, border.h);
}

MapWindow::MapWindow(const ant_proto::MapWindow& msg)
    : border(msg.border()), map(new TCODMap(border.w, border.h)) {
    for(const auto& room_msg : msg.rooms()) rooms.emplace_back(room_msg);
    for(const auto& corridor_msg : msg.corridors())
        corridors.emplace_back(corridor_msg);
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

void MapWindow::resize(long width, long height) {
    if(width <= 0 || height <= 0) {
        SPDLOG_WARN("Attempted to resize map window to {}x{}", width, height);
        return;
    }

    SPDLOG_INFO("Resizing map window to {}x{}", width, height);
    delete map;
    border = Rect(border.x1, border.y1, width, height);
    map = new TCODMap(width, height);
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

    // SPDLOG_TRACE("Computing FOV from ({}, {}) with radius {}", local_x,
    // local_y,
    //  radius);
    map->computeFov(local_x, local_y, radius);
    // SPDLOG_TRACE("FOV computed");
}

void MapWindow::set_wall(long x, long y) {
    long local_x, local_y;
    bool is_valid;
    to_local_coords(x, y, local_x, local_y, is_valid);
    if(!is_valid) {
        SPDLOG_ERROR("Invalid coordinates ({}, {}) when set wall", x, y);
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
        SPDLOG_ERROR("Invalid coordinates ({}, {}) when set floor", x, y);
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

ant_proto::MapWindow MapWindow::get_proto() const {
    ant_proto::MapWindow msg;
    msg.set_room_count(rooms.size());
    msg.set_corridor_count(corridors.size());
    *msg.mutable_border() = border.get_proto();
    for(const auto& room : rooms) *msg.add_rooms() = room.get_proto();
    for(const auto& corridor : corridors)
        *msg.add_corridors() = corridor.get_proto();
    return msg;
}
