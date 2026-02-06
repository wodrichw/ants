#pragma once

#include <cstdint>
#include <vector>

#include "ui/colors.hpp"
#include "ui/render.hpp"

class TestRenderer : public Renderer {
   public:
    enum class RenderMode { DEFAULT, SCENT };

    struct TileState {
        char ch = ' ';
        tcod::ColorRGB fg = color::white;
        tcod::ColorRGB bg = color::black;
    };

    void render_map(LayoutBox const&, Map& map, MapWindow const& window) override {
        ensure_tiles(window);
        for(long local_x = 0; local_x < window.border.w; ++local_x) {
            for(long local_y = 0; local_y < window.border.h; ++local_y) {
                long x = local_x + window.border.x1;
                long y = local_y + window.border.y1;
                TileState& tile = tiles[local_index(local_x, local_y)];
                tile.ch = ' ';
                tile.fg = color::white;
                tile.bg = render_mode == RenderMode::SCENT
                              ? get_scent_bg(map, x, y)
                              : get_default_bg(map, x, y);
            }
        }
    }

    void render_ant(LayoutBox const&, Map& map, EntityData& a,
                    MapWindow const& window) override {
        long local_x = 0;
        long local_y = 0;
        bool is_valid = false;
        window.to_local_coords(a.x, a.y, local_x, local_y, is_valid);
        if(!is_valid) return;
        if(!map.in_fov(a.x, a.y)) return;

        TileState& tile = tiles[local_index(local_x, local_y)];
        tile.ch = a.ch;
        tile.fg = a.col;
    }

    void render_building(LayoutBox const&, Building& b,
                         MapWindow const& window) override {
        for(long xi = b.border.x1; xi <= b.border.x2; ++xi) {
            for(long yi = b.border.y1; yi <= b.border.y2; ++yi) {
                long local_x = 0;
                long local_y = 0;
                bool is_valid = false;
                window.to_local_coords(xi, yi, local_x, local_y, is_valid);
                if(!is_valid) continue;
                TileState& tile = tiles[local_index(local_x, local_y)];
                tile.bg = b.color;
            }
        }
    }

    void render_text_editor(LayoutBox const&, TextEditor const&, ulong) override {}
    void render_help_boxes(LayoutBox const&) override {}
    void present() override {}
    void pixel_to_tile_coordinates(int, int, long& tile_x, long& tile_y) override {
        tile_x = 0;
        tile_y = 0;
    }

    void use_default_tile_rendering() override {
        render_mode = RenderMode::DEFAULT;
        scent_index = 0;
    }

    void use_scent_tile_rendering(ulong scent_idx) override {
        render_mode = RenderMode::SCENT;
        scent_index = scent_idx;
    }

    RenderMode get_render_mode() const { return render_mode; }
    ulong get_scent_index() const { return scent_index; }

    bool has_tile(long world_x, long world_y) const {
        if(!has_window) return false;
        long local_x = world_x - last_window_border.x1;
        long local_y = world_y - last_window_border.y1;
        return local_x >= 0 && local_x < last_window_border.w && local_y >= 0 &&
               local_y < last_window_border.h;
    }

    tcod::ColorRGB get_tile_bg(long world_x, long world_y) const {
        if(!has_tile(world_x, world_y)) return color::black;
        long local_x = world_x - last_window_border.x1;
        long local_y = world_y - last_window_border.y1;
        return tiles[local_index(local_x, local_y)].bg;
    }

   private:
    void ensure_tiles(MapWindow const& window) {
        if(!has_window || window.border.w != last_window_border.w ||
           window.border.h != last_window_border.h ||
           window.border.x1 != last_window_border.x1 ||
           window.border.y1 != last_window_border.y1) {
            last_window_border = window.border;
            tiles.assign(static_cast<size_t>(window.border.w * window.border.h),
                         TileState{});
            has_window = true;
        }
    }

    size_t local_index(long local_x, long local_y) const {
        return static_cast<size_t>(local_y * last_window_border.w + local_x);
    }

    tcod::ColorRGB get_default_bg(Map& map, long x, long y) const {
        const tcod::ColorRGB dark_wall = color::light_black;
        const tcod::ColorRGB dark_ground = color::dark_grey;
        const tcod::ColorRGB light_wall = color::indian_red;
        const tcod::ColorRGB light_ground = color::grey;

        if(map.in_fov(x, y)) {
            return map.is_wall(x, y) ? light_wall : light_ground;
        }

        return map.is_wall(x, y) || !map.is_explored(x, y) ? dark_wall
                                                           : dark_ground;
    }

    tcod::ColorRGB get_scent_bg(Map& map, long x, long y) const {
        const tcod::ColorRGB dark_wall = color::light_black;
        const tcod::ColorRGB dark_ground = color::dark_grey;
        const tcod::ColorRGB light_wall = color::indian_red;

        ulong scents = map.get_tile_scents_by_coord(x, y);
        unsigned char scent =
            static_cast<unsigned char>(static_cast<signed char>(scents >> scent_index));
        const tcod::ColorRGB scent_color{scent, scent, scent};

        if(map.in_fov(x, y)) {
            return map.is_wall(x, y) ? light_wall : scent_color;
        }

        return map.is_wall(x, y) || !map.is_explored(x, y) ? dark_wall
                                                           : dark_ground;
    }

    RenderMode render_mode = RenderMode::DEFAULT;
    ulong scent_index = 0;
    Rect last_window_border = {};
    bool has_window = false;
    std::vector<TileState> tiles = {};
};
