#ifndef __BUILDING_HPP
#define __BUILDING_HPP

#include <libtcod/color.hpp>
#include "map.hpp"
#include "colors.hpp"

class Building {
protected:
public:
    Building(int x, int y, int w, int h, int id, tcod::ColorRGB color, Map& map):
        x(x), y(y), w(w), h(h), id(id), color(color)
    {
        // Mark building location in map
        for( int xi = x; xi < x + w; ++xi ) {
            for( int yi = y; yi < y + h; ++yi ) {
                map.getTile(xi, yi).bldgId.emplace(id);
            }
        }
    }
    const int x, y, w, h, id; // bottom left (x, y), width, height
    const tcod::ColorRGB color;
};

class Nursery : public Building {
public:
    Nursery(int x, int y, int id, Map& map): Building(x, y, 3, 3, id, color::blue, map) {}

};



#endif // __BUILDING_HPP
