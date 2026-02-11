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
void LayoutBox::resize(long new_x, long new_y, long new_w, long new_h) {
    x = new_x;
    y = new_y;
    w = new_w;
    h = new_h;
    SPDLOG_TRACE("Resized box to ({}, {}) with dimensions {}x{}", x, y, w, h);
}
std::pair<LayoutBox *, LayoutBox *> &LayoutBox::split(ulong percentage,
                                                      Orientation orientation) {
    if(orientation == Orientation::HORIZONTAL) {
        SPDLOG_DEBUG("Splitting box horizontally at {}%", percentage);
        long first_h = (h * static_cast<long>(percentage)) / 100;
        long second_h = h - first_h;
        children.first = new LayoutBox(x, y, w, first_h);
        children.second = new LayoutBox(x, y + first_h, w, second_h);

    } else {
        SPDLOG_DEBUG("Splitting box vertically at {}%", percentage);
        long first_w = (w * static_cast<long>(percentage)) / 100;
        long second_w = w - first_w;
        children.first = new LayoutBox(x, y, first_w, h);
        children.second = new LayoutBox(x + first_w, y, second_w, h);
    }
    SPDLOG_TRACE("Layout box splitting completed");
    return children;
}

BoxManager::BoxManager(ulong w, ulong h)
        : main(static_cast<long>(w), static_cast<long>(h)),
            text_editor_root(static_cast<long>(w), static_cast<long>(h)) {
    SPDLOG_DEBUG("Creating BoxManager");
    ulong map_split = 60;

    SPDLOG_DEBUG("Splitting main box to create map and sidebar");
    std::tie(map_box, sidebar_box) =
        main.split(sidebar_split_percent, LayoutBox::Orientation::HORIZONTAL);

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

    set_sidebar_expanded(false);

    SPDLOG_TRACE("BoxManager created");
}

void BoxManager::toggle_sidebar() {
    set_sidebar_expanded(!sidebar_expanded);
}

void BoxManager::set_sidebar_expanded(bool expanded) {
    sidebar_expanded = expanded;
    long main_w = main.get_width();
    long main_h = main.get_height();

    if(sidebar_expanded) {
        long map_h = (main_h * static_cast<long>(sidebar_split_percent)) / 100;
        long sidebar_h = main_h - map_h;
        map_box->resize(0, 0, main_w, map_h);
        sidebar_box->resize(0, map_h, main_w, sidebar_h);
        SPDLOG_DEBUG("Sidebar expanded: map {}x{}, sidebar {}x{}", main_w,
                     map_h, main_w, sidebar_h);
    } else {
        map_box->resize(0, 0, main_w, main_h);
        sidebar_box->resize(0, main_h, main_w, 0);
        SPDLOG_DEBUG("Sidebar collapsed: map {}x{}", main_w, main_h);
    }
}

void LayoutBox::center(ulong new_width, ulong new_height) {
    long width = static_cast<long>(new_width);
    long height = static_cast<long>(new_height);
    x = xp + (wp - width) / 2;
    y = yp + (hp - height) / 2;
    w = width;
    h = height;
    SPDLOG_TRACE("Centered box at ({}, {}) with new dimensions {}x{}", x, y, w,
                 h);
}
