#pragma once

#include <vector>

#include "entity/rect.hpp"
#include "spdlog/spdlog.h"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;

struct MapSectionData {
    Rect border;
    std::vector<Rect> rooms;
    std::vector<Rect> corridors;

    MapSectionData(): border() {}
    MapSectionData(Rect const& border) : border(border) {}
    MapSectionData(MapSectionData const& other): border(other.border) {
        for (auto &room : other.rooms) {
            rooms.push_back(room);
        }

        for (auto &corridor : other.corridors) {
            corridors.push_back(corridor);
        }
    }

    MapSectionData& operator=(MapSectionData const& other) {
        border = other.border;
        rooms.clear();
        corridors.clear();

        for (auto &room : other.rooms) {
            rooms.push_back(room);
        }

        for (auto &corridor : other.corridors) {
            corridors.push_back(corridor);
        }

        return *this;
    }
};
