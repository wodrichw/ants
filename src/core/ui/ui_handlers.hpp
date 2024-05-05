#pragma once

#include "entity/map.hpp"
#include "ui/render.hpp"
#include "ui/subscriber.hpp"
#include "entity/entity_manager.hpp"

class MoveLeftHandler: public Subscriber<KeyboardEvent> {
    Map& map;
    MapEntity& entity;
public:
    MoveLeftHandler(Map& map, MapEntity& entity):
        map(map),
        entity(entity)
    {}

    void operator()(KeyboardEvent const&) { 
        map.move_entity(entity, -1, 0);
    }
};

class MoveRightHandler: public Subscriber<KeyboardEvent> {
    Map& map;
    MapEntity& entity;
public:
    MoveRightHandler(Map& map, MapEntity& entity):
        map(map),
        entity(entity)
    {}

    void operator()(KeyboardEvent const&) {
        map.move_entity(entity, 1, 0);
    }
};

class MoveUpHandler: public Subscriber<KeyboardEvent> {
    Map& map;
    MapEntity& entity;
public:
    MoveUpHandler(Map& map, MapEntity& entity):
        map(map),
        entity(entity)
    {}

    void operator()(KeyboardEvent const&) {
        map.move_entity(entity, 0, -1);
    }
};

class MoveDownHandler: public Subscriber<KeyboardEvent> {
    Map& map;
    MapEntity& entity;
public:
    MoveDownHandler(Map& map, MapEntity& entity):
        map(map),
        entity(entity)
    {}

    void operator()(KeyboardEvent const&) {
        map.move_entity(entity, 0, 1);
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

