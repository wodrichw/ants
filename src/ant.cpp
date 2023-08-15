#include <libtcod.hpp>
#include <iostream>
#include <libtcod/color.hpp>
#include "ant.hpp"


namespace ant {


    Ant::Ant(int x, int y, int fovRadius, char ch, tcod::ColorRGB col):
        x(x), y(y), fovRadius(fovRadius), ch(ch), col(col)
    {}

    Player::Player(int x, int y, int fovRadius, char ch, tcod::ColorRGB col): Ant(x, y, fovRadius, ch, col) {}

    void Player::updatePositionByDelta(int dx, int dy)
    {
        x += dx;
        y += dy;
    }

}
