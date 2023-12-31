#ifndef __ANT_HPP
#define __ANT_HPP

#include <libtcod/color.hpp>


namespace ant {

class Ant { 
public :
    int x, y;
    int fovRadius;
    char ch;
    tcod::ColorRGB col;
    std::optional<int> bldgId; // the building the player is occupying. Will not be set if player not in a building.

    Ant(int x, int y, int fovRadius, char ch, tcod::ColorRGB col);
    void updatePositionByDelta(int dx, int dy);

    virtual bool isInFov() = 0;
    virtual void resetFov() { return; }

    virtual ~Ant() = default;
};


class Player: public Ant {
public:
    Player(int x, int y, int fovRadius, char ch, tcod::ColorRGB col);
    bool isInFov() { return true; }
};


class Worker: public Ant {
public:
    Worker(int x, int y);
    bool isInFov() { return true; }
};

}
#endif //__ANT_HPP
