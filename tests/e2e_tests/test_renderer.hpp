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
    void render_sidebar(LayoutBox const& box, SidebarMenu const& menu,
                        ClockSpeed) override {
        sidebar_rendered = true;
        render_sidebar_tiles(box, menu);
    }
    void render_toggle_button(bool expanded) override {
        toggle_button_rendered = true;
        toggle_button_expanded = expanded;
    }
    void present() override {}
    void pixel_to_tile_coordinates(int, int, long& tile_x, long& tile_y) override {
        tile_x = pixel_tile_x;
        tile_y = pixel_tile_y;
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

    bool was_sidebar_rendered() const { return sidebar_rendered; }
    bool was_toggle_button_rendered() const { return toggle_button_rendered; }
    bool get_toggle_button_expanded() const { return toggle_button_expanded; }
    bool has_sidebar_tiles() const { return has_sidebar_tiles_; }
    long get_sidebar_width() const { return sidebar_w; }
    long get_sidebar_height() const { return sidebar_h; }
    tcod::ColorRGB get_sidebar_bg(long x, long y) const {
        if(!has_sidebar_tile(x, y)) return color::black;
        return sidebar_tiles[sidebar_index(x, y)].bg;
    }
    char get_sidebar_ch(long x, long y) const {
        if(!has_sidebar_tile(x, y)) return ' ';
        return sidebar_tiles[sidebar_index(x, y)].ch;
    }
    void reset_render_flags() {
        sidebar_rendered = false;
        toggle_button_rendered = false;
    }

    long pixel_tile_x = 0;
    long pixel_tile_y = 0;

   private:
    void render_sidebar_tiles(LayoutBox const& box, SidebarMenu const& menu) {
        long w = box.get_width();
        long h = box.get_height();
        if(w <= 3 || h <= 3) {
            has_sidebar_tiles_ = false;
            sidebar_tiles.clear();
            sidebar_w = 0;
            sidebar_h = 0;
            return;
        }

        sidebar_w = w;
        sidebar_h = h;
        sidebar_tiles.assign(static_cast<size_t>(w * h), TileState{});
        has_sidebar_tiles_ = true;

        std::vector<std::string> asciiGrid(static_cast<size_t>(h));
        for(long i = 0; i < h; ++i) {
            asciiGrid[static_cast<size_t>(i)] =
                std::string(static_cast<size_t>(w), ' ');
        }

        std::vector<std::string> lines;
        const auto& items = menu.items();
        for(size_t i = 0; i < items.size(); ++i) {
            bool is_selected = !menu.is_submenu_open() &&
                               menu.selected_index() == i;
            bool is_open = menu.open_submenu_index().has_value() &&
                           menu.open_submenu_index().value() == i;

            std::string label = items[i].label;
            if(!items[i].children.empty()) {
                label += is_open ? " v" : " >";
            }

            lines.push_back(std::string(is_selected ? "> " : "  ") + label);

            if(is_open) {
                const auto& submenu = items[i].children;
                for(size_t j = 0; j < submenu.size(); ++j) {
                    bool is_sub_selected = menu.submenu_selected_index() == j;
                    std::string sub_label = submenu[j].label;
                    lines.push_back(
                        std::string(is_sub_selected ? "  > " : "    ") +
                        sub_label);
                }
            }
        }

        populate_box(asciiGrid, 1, 1, static_cast<int>(w - 2),
                 static_cast<int>(h - 2), lines);

        for(long y = 0; y < h; ++y) {
            for(long x = 0; x < w; ++x) {
                TileState& tile = sidebar_tiles[sidebar_index(x, y)];
                tile.ch = asciiGrid[static_cast<size_t>(y)]
                                       [static_cast<size_t>(x)];
                tile.fg = color::white;
                tile.bg = color::dark_grey;
            }
        }
    }

    void populate_box(std::vector<std::string>& asciiGrid, long x, long y,
                      int w, int h, const std::vector<std::string>& text,
                      ushort offset_x = 0, ushort offset_y = 0) {
        if(w <= 1 || h <= 1) return;
        auto populate_char = [&](long x_idx, long y_idx, char ch) {
            asciiGrid[static_cast<size_t>(y + y_idx)]
                     [static_cast<size_t>(x + x_idx)] = ch;
        };

        populate_char(0, 0, '+');
        populate_char(0, h - 1, '+');
        populate_char(w - 1, 0, '+');
        populate_char(w - 1, h - 1, '+');
        for(long i = 1; i < h - 1; ++i) {
            populate_char(0, i, '|');
            populate_char(w - 1, i, '|');
        }
        for(long i = 1; i < w - 1; ++i) {
            populate_char(i, 0, '-');
            populate_char(i, h - 1, '-');
        }

        for(long i = 1; i < h - 1; ++i) {
            auto text_line_idx = static_cast<size_t>(offset_y + i - 1);
            bool is_line_filled = text_line_idx < text.size();
            for(long j = 1; j < w - 1; ++j) {
                auto char_idx = static_cast<size_t>(offset_x + j - 1);
                populate_char(
                    j, i,
                    is_line_filled && char_idx < text[text_line_idx].size()
                        ? text[text_line_idx][char_idx]
                        : ' ');
            }
        }
    }

    bool has_sidebar_tile(long x, long y) const {
        return has_sidebar_tiles_ && x >= 0 && y >= 0 && x < sidebar_w &&
               y < sidebar_h;
    }

    size_t sidebar_index(long x, long y) const {
        return static_cast<size_t>(y * sidebar_w + x);
    }
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
    bool sidebar_rendered = false;
    bool toggle_button_rendered = false;
    bool toggle_button_expanded = false;
    bool has_sidebar_tiles_ = false;
    long sidebar_w = 0;
    long sidebar_h = 0;
    std::vector<TileState> sidebar_tiles = {};
};
