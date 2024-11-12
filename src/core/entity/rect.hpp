#pragma once

#include "proto/utils.pb.h"

struct Rect {
    long x1 = 0, y1 = 0, w = 0, h = 0, x2 = 0, y2 = 0;
    long center_x = 0, center_y = 0;

    Rect() : x1(0), y1(0), w(0), h(0), x2(0), y2(0), center_x(0), center_y(0) {}
    Rect(long x1, long y1, long w, long h);
    Rect(Rect const& other);

    Rect(const ant_proto::Rect& msg);
    ant_proto::Rect get_proto() const;

    Rect& operator=(Rect const& other);

    bool is_inside(long x, long y) const;
    void set_top_left(long new_x1, long new_y1);
    void set_center(long new_center_x, long new_center_y);

    static Rect from_top_left(long x1, long y1, long w, long h);
    static Rect from_center(long center_x, long center_y, long w, long h);
    static Rect from_corners(long x1, long y1, long x2, long y2);
};

