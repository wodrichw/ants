#pragma once

#include <utility>

using ulong = unsigned long;

struct LayoutBox {
    enum Orientation { HORIZONTAL, VERTICAL };

   private:
    long const xp = 0, yp = 0, wp = 0, hp = 0;
    long x = 0, y = 0, w = 0, h = 0;  // absolute coordinates
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

    LayoutBox *map_box = nullptr, *sidebar_box = nullptr;
    LayoutBox *text_editor_content_box = nullptr,
              *text_editor_registers_box = nullptr;

   private:
    LayoutBox main;
    LayoutBox text_editor_root;
};
