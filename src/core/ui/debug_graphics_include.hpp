#pragma once

#include <math.h>

#include <functional>
#include <libtcod.hpp>

#include "app/globals.hpp"
#include "map/map.hpp"
#include "map/window.hpp"
#include "ui/layoutbox.hpp"

// make sure the values are positive for modulus operations
inline void debug_tile(const Map& map, TCOD_ConsoleTile& tile, long x, long y) {
    long constexpr chunk_inc = globals::CHUNK_LENGTH << 10;

    long pos_y = y;
    while(pos_y < 0) pos_y += chunk_inc;
    long y_mod = pos_y % globals::CHUNK_LENGTH;
    if(y_mod > 2) return;

    long pos_x = x;
    while(pos_x < 0) pos_x += chunk_inc;
    long x_mod = pos_x % globals::CHUNK_LENGTH;
    if(x_mod > 6) return;

    long true_x = x - x_mod;
    long true_y = y - y_mod;

    std::vector<ChunkMarker> cm =
        map.get_chunk_markers(Rect(true_x, true_y, 1, 1));
    ulong chunk_id = cm[0].id;

    if(x_mod == 0) {
        char id = y_mod == 0 ? 'C' : (y_mod == 1 ? 'X' : 'Y');
        tile.ch = id;
        return;
    }
    if(x_mod == 1) {
        tile.ch = ':';
        return;
    }

    long value = y_mod == 0
                     ? chunk_id
                     : (y_mod == 1 ? std::abs(true_x) : std::abs(true_y));
    if(x_mod == 2) {
        if(value >= 0) return;

        tile.ch = '-';
        return;
    }

    long power = 1000;
    for(long i = 3; i < x_mod; ++i) power /= 10;

    char digit = '0' + (value / power) % 10;

    tile.ch = digit;
}

inline void debug_chunks(
    LayoutBox const& box, Map const& map, MapWindow const& window,
    std::function<TCOD_ConsoleTile&(LayoutBox const&, long, long)> get_tile,
    bool is_debug_graphics) {
    if(!is_debug_graphics) return;
    for(long local_x = 0; local_x < window.border.w; ++local_x) {
        for(long local_y = 0; local_y < window.border.h; ++local_y) {
            long x = local_x + window.border.x1;
            long y = local_y + window.border.y1;
            auto& tile = get_tile(box, local_x, local_y);
            debug_tile(map, tile, x, y);
        }
    }
}
