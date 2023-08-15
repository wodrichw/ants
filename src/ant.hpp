#ifndef __ANT_HPP
#define __ANT_HPP

#include <cwchar>
#include <libtcod.hpp>
#include <libtcod/color.hpp>
#include <string>


namespace ant {

class Ant { 
public :
    int x, y;
    int fovRadius;
    char ch;
    tcod::ColorRGB col;

    Ant(int x, int y, int fovRadius, char ch, tcod::ColorRGB col);
    virtual bool isInFov() = 0;
    virtual void resetFov() { return; }

    virtual ~Ant() = default;
};


class Player :public Ant {
public:
    Player(int x, int y, int fovRadius, char ch, tcod::ColorRGB col);
    bool isInFov() { return true; }
    void updatePositionByDelta(int dx, int dy);
};

}
#endif //__ANT_HPP
