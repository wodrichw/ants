#pragma once

#include <functional>
#include <spdlog/spdlog.h>

#include "entity/entity_actions.hpp"
#include "entity/entity_manager.hpp"
#include "map/manager.hpp"
#include "map/map.hpp"
#include "map/world.hpp"
#include "ui/render.hpp"
#include "ui/sidebar_menu.hpp"
#include "ui/subscriber.hpp"

struct EngineState;

class MoveHandler : public Subscriber<KeyboardEvent> {
    MapEntity& entity;
    MapManager& map_manager;
    MapWorld& map_world;
    const ulong& entity_depth;
    bool move_only_on_current_depth;
    long dx = {}, dy = {};
    std::function<bool()> is_input_blocked;

   public:
    MoveHandler(MapEntity& entity, MapManager& map_manager, MapWorld& map_world,
                ulong& entity_depth, bool move_only_on_current_depth, long dx,
                long dy, std::function<bool()> is_input_blocked = nullptr);

    void operator()(KeyboardEvent const&);
};

class ChangeLevelHandler : public Subscriber<KeyboardEvent> {
   public:
    MapManager& map_manager;
    enum Direction { UP, DOWN } dir;

    ChangeLevelHandler(MapManager& map_manager, Direction dir);
    void operator()(KeyboardEvent const&);
};

class DigHandler : public Subscriber<KeyboardChordEvent> {
    MapEntity& entity;
    MapWorld& map_world;
    Inventory& inventory;
    long dx = {}, dy = {};

   public:
    DigHandler(MapEntity& entity, MapWorld& map_world, Inventory& inventory,
               long dx, long dy);

    void operator()(KeyboardChordEvent const&);
};

class ClickHandler : public Subscriber<MouseEvent> {
    MapWorld& map_world;
    Renderer& renderer;

   public:
    ClickHandler(MapWorld& map_world, Renderer& renderer);
    void operator()(MouseEvent const& event);
};

class CreateAntHandler : public Subscriber<KeyboardEvent> {
    EntityManager& entity_manager;
    HardwareManager& hardware_manager;
    SoftwareManager& software_manager;

   public:
    CreateAntHandler(EntityManager& entity_manager,
                     HardwareManager& hardware_manager,
                     SoftwareManager& software_manager);

    void operator()(KeyboardEvent const&);
};

class ReloadGameHandler : public Subscriber<KeyboardEvent> {
   public:
    ReloadGameHandler(bool& is_reload_game);
    void operator()(KeyboardEvent const&);

   private:
    bool& is_reload_game;
};

class DefaultMapTileRendererHandler : public Subscriber<KeyboardEvent> {
    Renderer& renderer;

   public:
    DefaultMapTileRendererHandler(Renderer& renderer);
    void operator()(KeyboardEvent const&);
};

class ScentMapTileRendererHandler : public Subscriber<KeyboardEvent> {
    Renderer& renderer;

   public:
    ScentMapTileRendererHandler(Renderer& renderer);
    void operator()(KeyboardEvent const& event);
};

class SidebarToggleHandler : public Subscriber<KeyboardEvent> {
    EngineState& engine;

   public:
    SidebarToggleHandler(EngineState& engine);
    void operator()(KeyboardEvent const&);
};

class SidebarMouseToggleHandler : public Subscriber<MouseEvent> {
    EngineState& engine;
    Renderer& renderer;

   public:
    SidebarMouseToggleHandler(EngineState& engine, Renderer& renderer);
    void operator()(MouseEvent const& event);
};

class SidebarNavHandler : public Subscriber<KeyboardEvent> {
    EngineState& engine;
    SidebarMenu& menu;

   public:
    SidebarNavHandler(EngineState& engine, SidebarMenu& menu);
    void operator()(KeyboardEvent const& event);
};
