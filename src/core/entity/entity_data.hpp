#pragma once
#include <libtcod/color.hpp>

using ulong = unsigned long;

struct RenderPosition {
    long x = 0, y = 0;
    bool requires_update = false;
};

class Packer;
class Unpacker;

using uchar = unsigned char;

struct EntityData {
   public:
    long x = 0, y = 0;

    char ch = '\0';
    long fov_radius = 0;
    tcod::ColorRGB col = {};
    RenderPosition last_rendered_pos = {};

    EntityData(char ch, long fov_radius, tcod::ColorRGB col);
    EntityData(long x, long y, char ch, long fov_radius, tcod::ColorRGB col);
    EntityData(Unpacker&);
    ~EntityData() = default;

    friend Packer& operator<<(Packer& p, EntityData const& obj);
};

enum MapEntityType { PLAYER, WORKER };

struct EntityMoveUpdate {
    long x, y, new_x, new_y;
    ulong abs_scents[4];
};

struct MapEntity {
    virtual ~MapEntity(){};
    virtual EntityData& get_data() = 0;
    virtual void move_callback(EntityMoveUpdate const&) = 0;
    virtual void click_callback(long x, long y) = 0;
    virtual void request_move() = 0;
    virtual void handle_empty_space(uchar bits) = 0;
    virtual void handle_full_space(uchar bits) = 0;
    virtual MapEntityType get_type() const = 0;
    friend Packer& operator<<(Packer& p, MapEntity const&) { return p; }
};