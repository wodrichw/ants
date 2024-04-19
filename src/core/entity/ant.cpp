#include "ant.hpp"

#include <libtcod/color.hpp>
#include <optional>

#include "ui/colors.hpp"
#include "spdlog/spdlog.h"

Ant::Ant(Map* map, long x, long y, int fovRadius, char ch, tcod::ColorRGB col)
    : x(x), y(y), fovRadius(fovRadius), ch(ch), col(col), bldgId(), map(map) {
        SPDLOG_DEBUG("Ant created at ({}, {})", x, y);
        SPDLOG_TRACE("Ant FOV radius: {} char: {} building id: {}", fovRadius, ch, bldgId.has_value() ? bldgId.value() : -1);
    }

void Ant::move(long dx, long dy) {
    SPDLOG_DEBUG("Ant moving from ({}, {}) to ({}, {})", x, y, x + dx, y + dy);
    x += dx;
    y += dy;
    last_rendered_pos.requires_update = true;

    map->updateFov();

    if(map->getTile(x, y).bldgId.has_value()) {
        SPDLOG_DEBUG("Ant found building at ({}, {})", x, y);
        bldgId.emplace(map->getTile(x, y).bldgId.value());
    } else if (bldgId.has_value()) {
        SPDLOG_DEBUG("Ant left building at ({}, {})", x, y);
        bldgId.reset();
    }
    SPDLOG_TRACE("Ant completed movement to ({}, {})", x, y);
}

bool Ant::can_move(long dx, long dy) { return map->canWalk(x + dx, y + dy); }

Player::Player(Map* map, long x, long y, int fovRadius, char ch,
               tcod::ColorRGB col)
    : Ant(map, x, y, fovRadius, ch, col) {
        SPDLOG_INFO("Player created at ({}, {})", x, y);
    }

Worker::Worker(Map* map, ButtonController* button_controller,
               ButtonController::ButtonData const& data)
    : Ant(map, data.x, data.y, 10, 'w', color::light_green),
      button_controller(button_controller),
      button(button_controller->createButton(
          data, [&]() { return toggle_color(); },
          std::optional<tcod::ColorRGB>())) {
            SPDLOG_DEBUG("Worker created at ({}, {})", x, y);
          }

bool Worker::toggle_color() {
    if(col == color::light_green){
        col = color::dark_yellow;
        SPDLOG_DEBUG("Worker toggled color to dark yellow");
    }else{
        col = color::light_green;
        SPDLOG_DEBUG("Worker toggled color to light green");
    }
    return true;
}

void Worker::move(long dx, long dy) {
    Ant::move(dx, dy);
    button_controller->moveButton(button, dx, dy);
}

bool Worker::can_move(long dx, long dy) {
    return Ant::can_move(dx, dy) &&
           button_controller->canMoveButton(button, dx, dy);
}