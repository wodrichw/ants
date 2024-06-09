#pragma once

#include <utility>

using ulong = unsigned long;

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
