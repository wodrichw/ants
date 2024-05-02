#include <vector>

#include <libtcod.hpp>

#include "entity/map_builder.hpp"
#include "spdlog/spdlog.h"

using ulong = unsigned long;

MapBuilder::MapBuilder(int width, int height) :
      width(width),
      height(height),
      rooms(),
      corridors(),
      map(new TCODMap(width, height)) {
        SPDLOG_INFO("Creating map of size {}x{}", width, height);
      }

MapBuilder::~MapBuilder() {
    SPDLOG_INFO("Destroying map");
    delete map;
    SPDLOG_DEBUG("Map destroyed");
}

bool MapBuilder::is_wall(long x, long y) const {
    // SPDLOG_TRACE("Checking if wall at ({}, {}): {}", x, y, !map->isWalkable(x, y));
    return !map->isWalkable(x, y);
}

bool MapBuilder::can_place(long x, long y) const {
    SPDLOG_TRACE("Checking if can walk at ({}, {})", x, y);
    if(x < 0 || x >= width || y < 0 || y >= height){
        SPDLOG_DEBUG("Cannot walk - out of bounds");
        return false;
    }
    if(is_wall(x, y)) {
        SPDLOG_DEBUG("Cannot walk - wall");
        return false;
    }
    return true;
}

bool MapBuilder::in_fov(long x, long y) const {
    return map->isInFov(x, y);
}

void MapBuilder::compute_fov(long x, long y, long radius) {
    map->computeFov(x, y, radius);
}

void MapBuilder::create_room(RoomRect const& rect) {
    rooms.push_back(rect);
    dig(rect.x1, rect.y1, rect.x2, rect.y2);
}

void MapBuilder::create_corridor(RoomRect const& rect) {
    corridors.push_back(rect);
    dig(rect.x1, rect.y1, rect.x2, rect.y2);
}

void MapBuilder::set_wall(long x, long y) {
    SPDLOG_TRACE("Setting wall at ({}, {})", x, y);
    map->setProperties(x, y, false, false);
}

/*
 * dig makes a rectangle of map specified by two x,y points as walkable
 */
void MapBuilder::dig(long x1, long y1, long x2, long y2) {
    SPDLOG_TRACE("Digging from ({}, {}) to ({}, {})", x1, y1, x2, y2);
    if(x2 < x1) {
        int tmp = x2;
        x2 = x1;
        x1 = tmp;
    }
    if(y2 < y1) {
        int tmp = y2;
        y2 = y1;
        y1 = tmp;
    }
    SPDLOG_TRACE("Setting properties for tiles");
    for(int tilex = x1; tilex <= x2; tilex++) {
        for(int tiley = y1; tiley <= y2; tiley++) {
            map->setProperties(tilex, tiley, true, true);
        }
    }
    SPDLOG_TRACE("Digging complete");
}