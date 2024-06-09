#include "ui/layoutbox.hpp"

#include "app/globals.hpp"
#include "spdlog/spdlog.h"

LayoutBox::LayoutBox() : xp(0), yp(0), wp(0), hp(0), x(0), y(0), w(0), h(0) {
    SPDLOG_TRACE("Empty LayoutBox created");
}
LayoutBox::LayoutBox(long w, long h)
    : xp(0), yp(0), wp(w), hp(h), x(0), y(0), w(w), h(h) {
    SPDLOG_TRACE("LayoutBox created with dimensions {}x{}", w, h);
}
LayoutBox::LayoutBox(long x, long y, long w, long h)
    : xp(x), yp(y), wp(w), hp(h), x(x), y(y), w(w), h(h) {
    SPDLOG_TRACE("LayoutBox created at ({}, {}) with dimensions {}x{}", x, y, w,
                 h);
}

LayoutBox::~LayoutBox() {
    SPDLOG_DEBUG("Destructing LayoutBox");
    if(children.first) delete children.first;
    if(children.second) delete children.second;
    SPDLOG_TRACE("LayoutBox destructed");
}
void LayoutBox::get_abs_pos(long x0, long y0, long &x1, long &y1) const {
    x1 = x + x0;
    y1 = y + y0;
}

long LayoutBox::get_width() const { return this->w; }
long LayoutBox::get_height() const { return this->h; }
std::pair<LayoutBox *, LayoutBox *> &LayoutBox::split(ulong percentage,
                                                      Orientation orientation) {
    if(orientation == Orientation::HORIZONTAL) {
        SPDLOG_DEBUG("Splitting box horizontally at {}%", percentage);
        ulong first_h = h * percentage / 100;
        ulong second_h = h - first_h;
        children.first = new LayoutBox(x, y, w, first_h);
        children.second = new LayoutBox(x, y + first_h, w, second_h);

    } else {
        SPDLOG_DEBUG("Splitting box vertically at {}%", percentage);
        ulong first_w = w * percentage / 100;
        ulong second_w = w - first_w;
        children.first = new LayoutBox(x, y, first_w, h);
        children.second = new LayoutBox(x + first_w, y, second_w, h);
    }
    SPDLOG_TRACE("Layout box splitting completed");
    return children;
}

BoxManager::BoxManager(ulong w, ulong h) : main(w, h), text_editor_root(w, h) {
    SPDLOG_DEBUG("Creating BoxManager");
    ulong map_split = 80;

    SPDLOG_DEBUG("Splitting main box to create map and sidebar");
    std::tie(map_box, sidebar_box) =
        main.split(map_split, LayoutBox::Orientation::VERTICAL);

    LayoutBox *editor_right_menu = nullptr, *editor_empty = nullptr;

    SPDLOG_DEBUG("Splitting sidebar to create text editor and registers");
    std::tie(text_editor_content_box, editor_right_menu) =
        text_editor_root.split(map_split, LayoutBox::Orientation::VERTICAL);

    SPDLOG_DEBUG(
        "Splitting editor right menu to create text editor and registers");
    std::tie(text_editor_registers_box, editor_empty) =
        editor_right_menu->split(30, LayoutBox::Orientation::HORIZONTAL);

    SPDLOG_DEBUG("Centering text editor content box");
    text_editor_content_box->center(
        globals::TEXTBOXWIDTH + globals::REGBOXWIDTH,
        globals::TEXTBOXHEIGHT + globals::REGBOXHEIGHT);

    SPDLOG_TRACE("BoxManager created");
}

void LayoutBox::center(ulong new_width, ulong new_height) {
    x = xp + (wp - new_width) / 2;
    y = yp + (hp - new_height) / 2;
    w = new_width;
    h = new_height;
    SPDLOG_TRACE("Centered box at ({}, {}) with new dimensions {}x{}", x, y, w,
                 h);
}
