#pragma once
#include <stddef.h>

#include <libtcod.hpp>

#include "entity/building.hpp"
#include "entity/entity_data.hpp"
#include "map/map.hpp"
#include "map/window.hpp"
#include "ui/layoutbox.hpp"
#include "ui/text_editor.hpp"
#include "spdlog/spdlog.h"

class Renderer {
   public:
    virtual void render_map(LayoutBox const&, Map const&, MapWindow const&) = 0;
    virtual void render_ant(LayoutBox const& box, Map& map, EntityData& a,
                            MapWindow const&) = 0;
    virtual void render_building(LayoutBox const& box, Building& b,
                                 MapWindow const&) = 0;
    virtual void render_text_editor(LayoutBox const& box,
                                    TextEditor const& editor,
                                    size_t ant_count) = 0;
    virtual void render_help_boxes(LayoutBox const& box) = 0;
    virtual void present() = 0;
    virtual void pixel_to_tile_coordinates(int pixel_x, int pixel_y,
                                           long& tile_x, long& tile_y) = 0;
    virtual void use_default_tile_rendering() = 0;
    virtual void use_scent_tile_rendering(ulong) = 0;
};

class NoneRenderer : public Renderer {
public:
    NoneRenderer() { SPDLOG_INFO("NoneRenderer initialized"); }
    void render_map(LayoutBox const&, Map const&, MapWindow const&){};
    void render_ant(LayoutBox const&, Map&, EntityData&, MapWindow const&){};
    void render_building(LayoutBox const&, Building&, MapWindow const&){};
    void render_text_editor(LayoutBox const&, TextEditor const&, size_t){};
    void render_help_boxes(LayoutBox const&){};
    void present(){};
    void pixel_to_tile_coordinates(int, int, long& tile_x, long& tile_y) {
        tile_x = 0;
        tile_y = 0;
    };
    void use_default_tile_rendering() {}
    void use_scent_tile_rendering(ulong) {}
};

struct MapTileRenderer {
    virtual ~MapTileRenderer() {}
    virtual void operator()(TCOD_ConsoleTile& tile, long x, long y)=0;
};

struct TcodMapTileRenderer : public MapTileRenderer {
    Map const& map;
    TcodMapTileRenderer(Map const& map);
    void operator()(TCOD_ConsoleTile& tile, long x, long y);
};

struct ScentMapTileRenderer : public MapTileRenderer {
    Map const& map;
    ulong scent_idx;
    ScentMapTileRenderer(Map const& map, ulong scent_idx);
    void operator()(TCOD_ConsoleTile& tile, long x, long y);
};

class tcodRenderer : public Renderer {
   public:
    tcodRenderer(bool is_debug_graphics);
    void render_map(LayoutBox const&, Map const&, MapWindow const&);
    void render_ant(LayoutBox const& box, Map& map, EntityData& a,
                    MapWindow const&);
    void render_building(LayoutBox const& box, Building& b, MapWindow const&);
    void render_text_editor(LayoutBox const& box, TextEditor const& editor,
                            size_t ant_count);
    void render_help_boxes(LayoutBox const&);
    void present();
    void pixel_to_tile_coordinates(int pixel_x, int pixel_y, long& tile_x,
                                   long& tile_y);
    void use_default_tile_rendering();
    void use_scent_tile_rendering(ulong);

   private:
    TCOD_ConsoleTile& get_tile(LayoutBox const& box, long x, long y);
    TCOD_ConsoleTile& clear_tile(LayoutBox const& box, long x, long y);
    const std::array<int, 4> get_rect(LayoutBox const& box, long x, long y,
                                      int w, int h);

    bool is_debug_graphics = false;
    tcod::Context context;
    tcod::Console root_console;
    std::function<std::unique_ptr<MapTileRenderer>(Map const&)> generate_tile_renderer;
};
