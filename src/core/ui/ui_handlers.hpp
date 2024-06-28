#pragma once

#include "entity/map.hpp"
#include "ui/render.hpp"
#include "ui/subscriber.hpp"
#include "entity/entity_manager.hpp"
#include "entity/entity_actions.hpp"

class MoveHandler: public Subscriber<KeyboardEvent> {
    Map& map;
    MapEntity& entity;
    long dx, dy;
public:
    MoveHandler(Map& map, MapEntity& entity, long dx, long dy):
        map(map), entity(entity), dx(dx), dy(dy) {}

    void operator()(KeyboardEvent const&) { 
        map.move_entity(entity, dx, dy);
    }
};

class DigHandler: public Subscriber<KeyboardChordEvent> {
    Map& map;
    MapEntity& entity;
    Inventory& inventory;
    long dx, dy;
public:
    DigHandler(Map& map, MapEntity& entity, Inventory& inventory, long dx, long dy):
        map(map), entity(entity), inventory(inventory), dx(dx), dy(dy)
    {}

    void operator()(KeyboardChordEvent const&) { 
        handle_dig(map, entity, inventory, dx, dy);
    }
};
class ClickHandler: public Subscriber<MouseEvent> {
    Map& map;
    Renderer& renderer;
public:
    ClickHandler(Map& map, Renderer& renderer):
        map(map),
        renderer(renderer)
    {}

    void operator()(MouseEvent const& event) { 
        long x = 0, y = 0;
        renderer.pixel_to_tile_coordinates(event.x, event.y, x, y);
        map.click(x, y);
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
