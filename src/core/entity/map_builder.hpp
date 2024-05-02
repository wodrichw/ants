#pragma once

#include <libtcod.hpp>
#include <libtcod/console_types.hpp>
#include <vector>

#include "spdlog/spdlog.h"

struct RoomRect {
    long x1, y1, w, h, x2, y2;
    long center_x, center_y;
    RoomRect(long x1, long y1, long w, long h) : x1(x1), y1(y1), w(w), h(h),
        x2(x1 + w - 1), y2(y1 + h - 1),
        center_x(x1 + w / 2), center_y(y1 + h / 2) {
            SPDLOG_TRACE("New Room Rect: ({}, {}) -> {}x{}", x1, y1, w, h);
        }
    
    static RoomRect from_top_left(long x1, long y1, long w, long h) {
        SPDLOG_TRACE("Creating room from top left: {}, {}, {}, {}", x1, y1, w, h);
        return RoomRect(x1, y1, w, h);
    }
    
    static RoomRect from_center(long center_x, long center_y, long w, long h) {
        SPDLOG_TRACE("Creating room from center: {}, {}, {}, {}", center_x, center_y, w, h);
        return RoomRect(center_x - w / 2, center_y - h / 2, w, h);
    }
    
    static RoomRect from_corners(long x1, long y1, long x2, long y2) {
        SPDLOG_TRACE("Creating room from corners: {}, {}, {}, {}", x1, y1, x2, y2);
        return RoomRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
    }
};

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;

class MapBuilder {
   public:
    int width, height;

    MapBuilder(int width, int height);
    ~MapBuilder();
    bool is_wall(long x, long y) const;
    bool can_place(long x, long y) const;
    bool in_fov(long x, long y) const;  // Fov = field of view
    void reset_fov(long x, long y);
    void compute_fov(long x, long y, long radius);
    void create_room(RoomRect const& room);
    void create_corridor(RoomRect const& corridor);
    void dig(long x1, long y1, long x2, long y2);
    RoomRect* get_first_room();

   private:
    std::vector<RoomRect> rooms, corridors;
    TCODMap* map;

    void set_wall(long x, long y);
};
