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

class DigLeftHandler: public Subscriber<KeyboardChordEvent> {
    Map& map;
    MapEntity& entity;
    Inventory& inventory;
public:
    DigLeftHandler(Map& map, MapEntity& entity, Inventory& inventory):
        map(map),
        entity(entity),
        inventory(inventory)
    {}

    void operator()(KeyboardChordEvent const&) { 
        ulong dirt_quantity = 0;
        if(inventory.max_space_for_item(DIRT) < dirt_quantity) return;
        if(!map.dig(entity, -1, 0)) return;


        inventory.add(DIRT, dirt_quantity);
        if (dirt_quantity > 0) {
            SPDLOG_ERROR("Failed to add dirt to inventory");
            return;
        }
    }
};

class DigRightHandler: public Subscriber<KeyboardChordEvent> {
    Map& map;
    MapEntity& entity;
    Inventory& inventory;
public:
    DigRightHandler(Map& map, MapEntity& entity, Inventory& inventory):
        map(map),
        entity(entity),
        inventory(inventory)
    {}

    void operator()(KeyboardChordEvent const&) {
        ulong dirt_quantity = 0;
        if(inventory.max_space_for_item(DIRT) < dirt_quantity){
            SPDLOG_DEBUG("Cannot dig since inventory cannot contain more dirt");
            return;
        }
        if(!map.dig(entity, 1, 0)) {
            return;
        }


        inventory.add(DIRT, dirt_quantity);
        if (dirt_quantity > 0) {
            SPDLOG_ERROR("Failed to add dirt to inventory");
            return;
        }
    }
};

class DigUpHandler: public Subscriber<KeyboardChordEvent> {
    Map& map;
    MapEntity& entity;
    Inventory& inventory;
public:
    DigUpHandler(Map& map, MapEntity& entity, Inventory& inventory):
        map(map),
        entity(entity),
        inventory(inventory)
    {}

    void operator()(KeyboardChordEvent const&) {
        ulong dirt_quantity = 0;
        if(inventory.max_space_for_item(DIRT) < dirt_quantity) return;
        if(!map.dig(entity, 0, -1)) return;


        inventory.add(DIRT, dirt_quantity);
        if (dirt_quantity > 0) {
            SPDLOG_ERROR("Failed to add dirt to inventory");
            return;
        }
    }
};

class DigDownHandler: public Subscriber<KeyboardChordEvent> {
    Map& map;
    MapEntity& entity;
    Inventory& inventory;
public:
    DigDownHandler(Map& map, MapEntity& entity, Inventory& inventory):
        map(map),
        entity(entity),
        inventory(inventory)
    {}

    void operator()(KeyboardChordEvent const&) {
        ulong dirt_quantity = 0;
        if(inventory.max_space_for_item(DIRT) < dirt_quantity) return;
        if(!map.dig(entity, 0, 1)) return;

        inventory.add(DIRT, dirt_quantity);
        if (dirt_quantity > 0) {
            SPDLOG_ERROR("Failed to add dirt to inventory");
            return;
        }
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

