#include "ui/ui_handlers.hpp"

#include "map/manager.hpp"

MoveHandler::MoveHandler(MapEntity& entity, MapManager& map_manager,
                         MapWorld& map_world, ulong& entity_depth,
                         bool move_only_on_current_depth, long dx, long dy)
    : entity(entity),
      map_manager(map_manager),
      map_world(map_world),
      entity_depth(entity_depth),
      move_only_on_current_depth(move_only_on_current_depth),
      dx(dx),
      dy(dy) {}

void MoveHandler::operator()(KeyboardEvent const&) {
    if(move_only_on_current_depth && map_world.current_depth != entity_depth)
        return;
    map_manager.move_entity(entity_depth, dy, dx, entity);
}

ChangeLevelHandler::ChangeLevelHandler(MapManager& map_manager, Direction dir)
    : map_manager(map_manager), dir(dir) {}

void ChangeLevelHandler::operator()(KeyboardEvent const&) {
    if(dir == Direction::UP) {
        map_manager.go_up();
    } else {
        map_manager.go_down();
    }
}

DigHandler::DigHandler(MapEntity& entity, MapWorld& map_world,
                       Inventory& inventory, long dx, long dy)
    : entity(entity),
      map_world(map_world),
      inventory(inventory),
      dx(dx),
      dy(dy) {}

void DigHandler::operator()(KeyboardChordEvent const&) {
    handle_dig(map_world.current_level().map, entity, inventory, dx, dy);
}

ClickHandler::ClickHandler(MapWorld& map_world, Renderer& renderer)
    : map_world(map_world), renderer(renderer) {}

void ClickHandler::operator()(MouseEvent const& event) {
    long x = 0, y = 0;
    renderer.pixel_to_tile_coordinates(event.x, event.y, x, y);
    map_world.current_level().map.click(x, y);
}

CreateAntHandler::CreateAntHandler(EntityManager& entity_manager,
                                   HardwareManager& hardware_manager,
                                   SoftwareManager& software_manager)
    : entity_manager(entity_manager),
      hardware_manager(hardware_manager),
      software_manager(software_manager) {}

void CreateAntHandler::operator()(KeyboardEvent const&) {
    entity_manager.create_ant(hardware_manager, software_manager);
}

ReloadGameHandler::ReloadGameHandler(bool& is_reload_game)
    : is_reload_game(is_reload_game) {}

void ReloadGameHandler::operator()(KeyboardEvent const&) {
    is_reload_game = true;
}

DefaultMapTileRendererHandler::DefaultMapTileRendererHandler(Renderer& renderer)
    : renderer(renderer) {}

void DefaultMapTileRendererHandler::operator()(KeyboardEvent const&) {
    renderer.use_default_tile_rendering();
}

ScentMapTileRendererHandler::ScentMapTileRendererHandler(Renderer& renderer)
    : renderer(renderer) {}

void ScentMapTileRendererHandler::operator()(KeyboardEvent const& event) {
    ulong scent_idx = event.type - KeyboardEventType::ONE_KEY_EVENT;
    renderer.use_scent_tile_rendering(scent_idx);
}
