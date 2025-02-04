#include "entity/rect.hpp"

#include "spdlog/spdlog.h"

Rect::Rect(long x1, long y1, long w, long h) : w(w), h(h) {
    SPDLOG_TRACE("New Room Rect: ({}, {}) -> {}x{}", x1, y1, w, h);
    set_top_left(x1, y1);
}

Rect::Rect(Rect const& other)
    : x1(other.x1),
      y1(other.y1),
      w(other.w),
      h(other.h),
      x2(other.x2),
      y2(other.y2),
      center_x(other.center_x),
      center_y(other.center_y) {}

Rect::Rect(const ant_proto::Rect& msg) : w(msg.w()), h(msg.h()) {
    set_top_left(msg.x1(), msg.y1());
    SPDLOG_TRACE("Unpacking rectangle - x: {} y: {} w: {} h: {}", msg.x1(),
                 msg.y1(), w, h);
}

Rect& Rect::operator=(Rect const& other) {
    x1 = other.x1;
    y1 = other.y1;
    w = other.w;
    h = other.h;
    x2 = other.x2;
    y2 = other.y2;
    center_x = other.center_x;
    center_y = other.center_y;
    return *this;
}

bool Rect::is_inside(long x, long y) const {
    return x >= x1 && x <= x2 && y >= y1 && y <= y2;
}

void Rect::set_top_left(long new_x1, long new_y1) {
    x1 = new_x1;
    y1 = new_y1;
    x2 = x1 + w - 1;
    y2 = y1 + h - 1;
    center_x = x1 + w / 2;
    center_y = y1 + h / 2;
}

void Rect::set_center(long new_center_x, long new_center_y) {
    center_x = new_center_x;
    center_y = new_center_y;
    x1 = center_x - w / 2;
    y1 = center_y - h / 2;
    x2 = center_x + w / 2;
    y2 = center_y + h / 2;
}

ant_proto::Rect Rect::get_proto() const {
    ant_proto::Rect msg;
    msg.set_x1(x1);
    msg.set_y1(y1);
    msg.set_w(w);
    msg.set_h(h);
    return msg;
}

Rect Rect::from_top_left(long x1, long y1, long w, long h) {
    SPDLOG_TRACE("Creating room from top left: {}, {}, {}, {}", x1, y1, w, h);
    return Rect(x1, y1, w, h);
}

Rect Rect::from_center(long center_x, long center_y, long w, long h) {
    SPDLOG_TRACE("Creating room from center: {}, {}, {}, {}", center_x,
                 center_y, w, h);
    return Rect(center_x - w / 2, center_y - h / 2, w, h);
}

Rect Rect::from_corners(long x1, long y1, long x2, long y2) {
    SPDLOG_TRACE("Creating room from corners: {}, {}, {}, {}", x1, y1, x2, y2);
    return Rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}
