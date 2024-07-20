#pragma once

#include <spdlog/spdlog.h>
#include "entity/map.hpp"
#include "entity/map_manager.hpp"
#include "ui/render.hpp"
#include "ui/subscriber.hpp"
#include "entity/entity_manager.hpp"
#include "entity/entity_actions.hpp"

class MoveHandler: public Subscriber<KeyboardEvent> {
    MapManager& map_manager;
    MapEntity& entity;
    const ulong& current_depth;
    const ulong& entity_depth;
    bool move_only_on_current_depth;
    long dx = {}, dy = {};
public:
    MoveHandler(
        MapManager& map_manager,
        MapEntity& entity,
        ulong& current_depth,
        ulong& entity_depth,
        bool move_only_on_current_depth,
        long dx, long dy
    ):
        map_manager(map_manager),
        entity(entity),
        current_depth(current_depth),
        entity_depth(entity_depth),
        move_only_on_current_depth(move_only_on_current_depth),
        dx(dx), dy(dy)
    {}

    void operator()(KeyboardEvent const&) {
        if (move_only_on_current_depth && current_depth != entity_depth) return;
        map_manager.get_map().move_entity(entity, dx, dy);
    }
};

class ChangeLevelHandler: public Subscriber<KeyboardEvent> {

public:
    EntityManager& entity_manager;
    enum Direction { UP, DOWN } dir;
    ChangeLevelHandler(EntityManager& entity_manager, Direction dir):
        entity_manager(entity_manager),
        dir(dir)
    {}

    void operator()(KeyboardEvent const&) {
        if (dir == Direction::UP) {
            entity_manager.go_up();
        } else {
            entity_manager.go_down();
        }
    }
};

class DigHandler: public Subscriber<KeyboardChordEvent> {
    MapManager& map_manager;
    MapEntity& entity;
    Inventory& inventory;
    long dx= {}, dy = {};
public:
    DigHandler(MapManager& map_manager, MapEntity& entity, Inventory& inventory, long dx, long dy):
        map_manager(map_manager), entity(entity), inventory(inventory), dx(dx), dy(dy)
    {}

    void operator()(KeyboardChordEvent const&) {
        handle_dig(map_manager.get_map(), entity, inventory, dx, dy);
    }
};

class ClickHandler: public Subscriber<MouseEvent> {
    MapManager& map_manager;
    Renderer& renderer;
public:
    ClickHandler(MapManager& map_manager, Renderer& renderer):
        map_manager(map_manager),
        renderer(renderer)
    {}

    void operator()(MouseEvent const& event) {
        long x = 0, y = 0;
        renderer.pixel_to_tile_coordinates(event.x, event.y, x, y);
        map_manager.get_map().click(x, y);
    }
};

class CreateAntHandler: public Subscriber<KeyboardEvent> {
    EntityManager& entity_manager;
    HardwareManager& hardware_manager;
    SoftwareManager& software_manager;
public:
    CreateAntHandler(EntityManager& entity_manager, HardwareManager& hardware_manager, SoftwareManager& software_manager):
        entity_manager(entity_manager), hardware_manager(hardware_manager), software_manager(software_manager) {}
    void operator()(KeyboardEvent const&) {
        entity_manager.create_ant(hardware_manager, software_manager);
    }
};

class ReloadGameHandler: public Subscriber<KeyboardEvent> {
    public:
    ReloadGameHandler(bool& is_reload_game) : is_reload_game(is_reload_game) {}
    void operator()(KeyboardEvent const&) { is_reload_game = true; }
    private:
    bool& is_reload_game;
};
