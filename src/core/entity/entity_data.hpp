#pragma once
#include <libtcod/color.hpp>
struct RenderPosition {
    long x = 0, y = 0;
    bool requires_update = false;
};

class Packer;
class Unpacker;

struct EntityData {
   public:
    long x, y;

    char ch;
    long fov_radius;
    tcod::ColorRGB col;
    RenderPosition last_rendered_pos;

    EntityData(char ch, long fov_radius, tcod::ColorRGB col);
    EntityData(long x, long y, char ch, long fov_radius, tcod::ColorRGB col);
    EntityData(Unpacker&);
    ~EntityData() = default;

    friend Packer& operator<<(Packer& p, EntityData const& obj);
};

enum MapEntityType { PLAYER, WORKER };

struct MapEntity {
    virtual ~MapEntity(){};
    virtual EntityData& get_data() = 0;
    virtual void move_callback(long x, long y, long new_x, long new_y) = 0;
    virtual void click_callback(long x, long y) = 0;
    virtual void request_move() = 0;
    virtual MapEntityType get_type() const = 0;
    friend Packer& operator<<(Packer& p, MapEntity const&) { return p; }
};