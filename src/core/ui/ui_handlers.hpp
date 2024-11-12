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
    );

    void operator()(KeyboardEvent const&);
};

class ChangeLevelHandler: public Subscriber<KeyboardEvent> {

public:
    EntityManager& entity_manager;
    enum Direction { UP, DOWN } dir;

    ChangeLevelHandler(EntityManager& entity_manager, Direction dir);
    void operator()(KeyboardEvent const&);
};

class DigHandler: public Subscriber<KeyboardChordEvent> {
    MapManager& map_manager;
    MapEntity& entity;
    Inventory& inventory;
    long dx= {}, dy = {};
public:
    DigHandler(MapManager& map_manager,
        MapEntity& entity,
        Inventory& inventory,
        long dx,
        long dy
    );

    void operator()(KeyboardChordEvent const&);
};

class ClickHandler: public Subscriber<MouseEvent> {
    MapManager& map_manager;
    Renderer& renderer;
public:
    ClickHandler(MapManager& map_manager, Renderer& renderer);
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
