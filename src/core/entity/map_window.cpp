#include <vector>

#include <libtcod.hpp>

#include "entity/map_window.hpp"
#include "spdlog/spdlog.h"

using ulong = unsigned long;

MapWindow::MapWindow(Rect const& border) :
      border(border),
      rooms(),
      corridors(),
      map(new TCODMap(border.w, border.h)) {
        SPDLOG_INFO("Creating map of size {}x{}", border.w, border.h);
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
    if (!is_valid) return;

    SPDLOG_TRACE("Computing FOV from ({}, {}) with radius {}", local_x, local_y, radius);
    map->computeFov(local_x, local_y, radius);
    SPDLOG_TRACE("FOV computed");
}

void MapWindow::set_wall(long x, long y) {
    long local_x, local_y;
    bool is_valid;
    to_local_coords(x, y, local_x, local_y, is_valid);
    if (!is_valid) {
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
    if (!is_valid) {
        // SPDLOG_ERROR("Invalid coordinates ({}, {}) when set floor", x, y);
        return;
    }

    // SPDLOG_TRACE("Setting floor at ({}, {})", local_x, local_y);
    map->setProperties(local_x, local_y, true, true);
}

void MapWindow::to_local_coords(long x, long y, long& local_x, long& local_y, bool& is_valid) const {
    local_x = x - border.x1;
    local_y = y - border.y1;
    is_valid = local_x >= 0 && local_x < border.w && local_y >= 0 && local_y < border.h;
}