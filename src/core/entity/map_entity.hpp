#pragma once
#include <libtcod/color.hpp>

struct RenderPosition {
    long x = 0, y = 0;
    bool requires_update = false;
};

struct EntityData {
   public:
    long x, y;

    char ch;
    long fov_radius;
    tcod::ColorRGB col;
    RenderPosition last_rendered_pos;

    EntityData(long x, long y, char ch, int fov_radius, tcod::ColorRGB col);
    ~EntityData()=default;
};

struct MapEntity {
    virtual EntityData& get_data() = 0;
    virtual void move_callback(long x, long y, long new_x, long new_y) = 0;
    virtual void click_callback(long x, long y) = 0;
    virtual ~MapEntity() {};
};