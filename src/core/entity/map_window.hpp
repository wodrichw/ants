#pragma once

#include <libtcod.hpp>
#include <libtcod/console_types.hpp>
#include <vector>

#include "entity/rect.hpp"

class MapWindow {
   public:
    Rect border;

    MapWindow(Rect const&);
    MapWindow(Unpacker&);
    ~MapWindow();
    void set_center(long x, long y);
    bool in_fov(long x, long y) const;  // Fov = field of view
    void compute_fov(long x, long y, long radius);

    void set_wall(long x, long y);
    void set_floor(long x, long y);
    void to_local_coords(long x, long y, long& local_x, long& local_y,
                         bool&) const;

    friend Packer& operator<<(Packer&, MapWindow const&);

   private:
    std::vector<Rect> rooms, corridors;
    TCODMap* map;
};
