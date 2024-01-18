#include <optional>
#include <libtcod/color.hpp>
#include "ant.hpp"
#include "colors.hpp"


namespace ant {


    Ant::Ant(int x, int y, int fovRadius, char ch, tcod::ColorRGB col):
        x(x), y(y), fovRadius(fovRadius), ch(ch), col(col), bldgId()
    {}

    void Ant::updatePositionByDelta(int dx, int dy)
    {
        x += dx;
        y += dy;
        last_rendered_pos.requires_update = true;
    }

    Player::Player(int x, int y, int fovRadius, char ch, tcod::ColorRGB col): 
        Ant(x, y, fovRadius, ch, col)
    {}

    Worker::Worker(int x, int y): 
        Ant(x, y, 10, 'w', color::light_green)
    {}


}
