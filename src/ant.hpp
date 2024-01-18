#pragma once

#include <libtcod/color.hpp>
#include <optional>

#include "brain.hpp"
#include "buttonController.hpp"
#include "map.hpp"
#include "operations.hpp"

struct PositionData {
    long x = 0, y = 0;
    bool requires_update = false;
};

class Ant {
public :
    long x, y, fovRadius;
    char ch;
    tcod::ColorRGB col;
    std::optional<int> bldgId; // the building the player is occupying. Will not be set if player not in a building.
    PositionData last_rendered_pos;

    Ant(Map* map, long x, long y, int fovRadius, char ch, tcod::ColorRGB col);
    bool can_move(long dx, long dy);
    void move(long dx, long dy);

    virtual bool isInFov() = 0;
    virtual void resetFov() { return; }

    virtual ~Ant() = default;
private:
    Map* map;
};


class Player: public Ant {
public:
    Player(Map* map, long x, long y, int fovRadius, char ch, tcod::ColorRGB col);
    bool isInFov() { return true; }
};

class Worker: public Ant {
public:
    Worker(Map* map, ButtonController* button_controller, ButtonController::ButtonData const& data);
    void move(long dx, long dy);
    bool can_move(long dx, long dy);
    bool isInFov() { return true; }
    bool toggle_color();

    Operations operations;
    DualRegisters cpu;
private:
    ButtonController* button_controller;
    ButtonController::Button* button;
    Map* map;
};