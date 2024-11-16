#pragma once

#include <vector>

#include "entity/rect.hpp"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;

struct MapSectionData {
    Rect border;
    std::vector<Rect> rooms;
    std::vector<Rect> corridors;

    MapSectionData(): border() {}
    MapSectionData(Rect const& border) : border(border) {}
    MapSectionData(MapSectionData const& other);

    MapSectionData& operator=(MapSectionData const& other);
};
