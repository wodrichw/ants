#include <optional>
#include <libtcod/color.hpp>

#include "ant.hpp"
#include "colors.hpp"

Ant::Ant(Map* map, long x, long y, int fovRadius, char ch, tcod::ColorRGB col):
    x(x), y(y), fovRadius(fovRadius), ch(ch), col(col), bldgId(), map(map)
{}

void Ant::move(long dx, long dy)
{
    x += dx;
    y += dy;
    last_rendered_pos.requires_update = true;

    map->updateFov();

    if (map->getTile(x, y).bldgId.has_value()) {
        bldgId.emplace(map->getTile(x, y).bldgId.value());
    } else {
        bldgId.reset();
    }
}

bool Ant::can_move(long dx, long dy)
{
    return map->canWalk(x + dx, y + dy);
}

Player::Player(Map* map, long x, long y, int fovRadius, char ch, tcod::ColorRGB col):
    Ant(map, x, y, fovRadius, ch, col)
{}

Worker::Worker(Map* map, ButtonController* button_controller, ButtonController::ButtonData const& data) : Ant(map, data.x, data.y, 10, 'w', color::light_green),
    button_controller(button_controller),
    button(button_controller->createButton(data, [&](){ return toggle_color(); }, std::optional<tcod::ColorRGB>()))
{}


bool Worker::toggle_color()
{
    if( col == color::light_green ) col = color::dark_yellow;
    else col = color::light_green;
    return true;
}

void Worker::move(long dx, long dy)
{
    Ant::move(dx, dy);
    button_controller->moveButton(button, dx, dy);
}

bool Worker::can_move(long dx, long dy)
{
    return Ant::can_move(dx, dy) && button_controller->canMoveButton(button, dx, dy);
}