#pragma once

#include <spdlog/spdlog.h>
#include "map/map.hpp"
#include "map/manager.hpp"
#include "map/world.hpp"
#include "ui/render.hpp"
#include "ui/subscriber.hpp"
#include "entity/entity_manager.hpp"
#include "entity/entity_actions.hpp"

class MoveHandler: public Subscriber<KeyboardEvent> {
    MapEntity& entity;
    MapWorld& map_world;
    const ulong& entity_depth;
    bool move_only_on_current_depth;
    long dx = {}, dy = {};
public:
    MoveHandler(
        MapEntity& entity,
        MapWorld& map_world,
        ulong& entity_depth,
        bool move_only_on_current_depth,
        long dx, long dy
    );

    void operator()(KeyboardEvent const&);
};

class ChangeLevelHandler: public Subscriber<KeyboardEvent> {

public:
    MapManager& map_manager;
    enum Direction { UP, DOWN } dir;

    ChangeLevelHandler(MapManager& map_manager, Direction dir);
    void operator()(KeyboardEvent const&);
};

class DigHandler: public Subscriber<KeyboardChordEvent> {
    MapEntity& entity;
    MapWorld& map_world;
    Inventory& inventory;
    long dx= {}, dy = {};
public:
    DigHandler(
        MapEntity& entity,
        MapWorld& map_world,
        Inventory& inventory,
        long dx,
        long dy
    );

    void operator()(KeyboardChordEvent const&);
};

class ClickHandler: public Subscriber<MouseEvent> {
    MapWorld& map_world;
    Renderer& renderer;
public:
    ClickHandler(MapWorld& map_world, Renderer& renderer);
    void operator()(MouseEvent const& event);
};

class CreateAntHandler: public Subscriber<KeyboardEvent> {
    EntityManager& entity_manager;
    HardwareManager& hardware_manager;
    SoftwareManager& software_manager;
public:
    CreateAntHandler(EntityManager& entity_manager,
        HardwareManager& hardware_manager,
        SoftwareManager& software_manager
    );

    void operator()(KeyboardEvent const&);
};

class ReloadGameHandler: public Subscriber<KeyboardEvent> {
    public:
    ReloadGameHandler(bool& is_reload_game);
    void operator()(KeyboardEvent const&);
    private:
    bool& is_reload_game;
};

class DefaultMapTileRendererHandler: public Subscriber<KeyboardEvent> {
    Renderer& renderer;
    public:
    DefaultMapTileRendererHandler(Renderer& renderer);
    void operator()(KeyboardEvent const&);
};

class ScentMapTileRendererHandler: public Subscriber<KeyboardEvent> {
    Renderer& renderer;
    public:
    ScentMapTileRendererHandler(Renderer& renderer);
    void operator()(KeyboardEvent const& event);
};
