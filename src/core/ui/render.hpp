#pragma once
#include <stddef.h>

#include <libtcod.hpp>

#include "entity/building.hpp"
#include "entity/map.hpp"
#include "entity/map_entity.hpp"
#include "entity/map_window.hpp"
#include "ui/text_editor.hpp"

struct LayoutBox {
    enum Orientation { HORIZONTAL, VERTICAL };

   private:
    long const xp, yp, wp, hp;
    long x, y, w, h;  // absolute coordinates
    // local coordinate: (0,0), (w,h) -> (x,y), (x+w,y+h)
    std::pair<LayoutBox*, LayoutBox*> children;

   public:
    LayoutBox();
    LayoutBox(long w, long h);
    LayoutBox(long x, long y, long w, long h);
    ~LayoutBox();

    void get_abs_pos(long x0, long y0, long& x1, long& y1) const;
    long get_width() const;
    long get_height() const;

    void center(ulong new_width, ulong new_height);

    std::pair<LayoutBox*, LayoutBox*>& split(ulong percentage,
                                             Orientation orientation);
};

struct BoxManager {
    BoxManager(ulong w, ulong h);

    LayoutBox *map_box, *sidebar_box;
    LayoutBox *text_editor_content_box, *text_editor_registers_box;

   private:
    LayoutBox main;
    LayoutBox text_editor_root;
};

class Renderer {
   public:
    virtual void renderMap(LayoutBox const&, Map const&, MapWindow const&) = 0;
    virtual void renderAnt(LayoutBox const& box, Map& map, EntityData& a,
                           MapWindow const&) = 0;
    virtual void renderBuilding(LayoutBox const& box, Building& b,
                                MapWindow const&) = 0;
    virtual void renderTextEditor(LayoutBox const& box,
                                  TextEditor const& editor,
                                  size_t ant_count) = 0;
    virtual void renderHelpBoxes(LayoutBox const& box) = 0;
    virtual void present() = 0;
    virtual void pixel_to_tile_coordinates(int pixel_x, int pixel_y,
                                           long& tile_x, long& tile_y) = 0;
};

class NoneRenderer : public Renderer {
   public:
    NoneRenderer() { SPDLOG_INFO("NoneRenderer initialized"); }
    void renderMap(LayoutBox const&, Map const&, MapWindow const&){};
    void renderAnt(LayoutBox const&, Map&, EntityData&, MapWindow const&){};
    void renderBuilding(LayoutBox const&, Building&, MapWindow const&){};
    void renderTextEditor(LayoutBox const&, TextEditor const&, size_t){};
    void renderHelpBoxes(LayoutBox const&){};
    void present(){};
    void pixel_to_tile_coordinates(int, int, long& tile_x, long& tile_y) {
        tile_x = 0;
        tile_y = 0;
    };
};

class tcodRenderer : public Renderer {
   public:
    tcodRenderer();
    void renderMap(LayoutBox const&, Map const&, MapWindow const&);
    void renderAnt(LayoutBox const& box, Map& map, EntityData& a,
                   MapWindow const&);
    void renderBuilding(LayoutBox const& box, Building& b, MapWindow const&);
    void renderTextEditor(LayoutBox const& box, TextEditor const& editor,
                          size_t ant_count);
    void renderHelpBoxes(LayoutBox const&);
    void present();
    void pixel_to_tile_coordinates(int pixel_x, int pixel_y, long& tile_x,
                                   long& tile_y);

   private:
    TCOD_ConsoleTile& get_tile(LayoutBox const& box, long x, long y);
    TCOD_ConsoleTile& clearCh(LayoutBox const& box, long x, long y);
    const std::array<int, 4> get_rect(LayoutBox const& box, long x, long y,
                                      int w, int h);

    tcod::Context context;
    tcod::Console root_console;
};
