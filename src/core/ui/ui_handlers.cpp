#include "ui/ui_handlers.hpp"

MoveHandler::MoveHandler(
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

void MoveHandler::operator()(KeyboardEvent const&) {
    if (move_only_on_current_depth && current_depth != entity_depth) return;
    map_manager.get_map().move_entity(entity, dx, dy);
}

ChangeLevelHandler::ChangeLevelHandler(EntityManager& entity_manager, Direction dir):
    entity_manager(entity_manager),
    dir(dir)
{}

void ChangeLevelHandler::operator()(KeyboardEvent const&) {
    if (dir == Direction::UP) {
        entity_manager.go_up();
    } else {
        entity_manager.go_down();
    }
}

DigHandler::DigHandler(MapManager& map_manager,
    MapEntity& entity,
    Inventory& inventory,
    long dx,
    long dy
):
    map_manager(map_manager),
    entity(entity),
    inventory(inventory),
    dx(dx),
    dy(dy)
{}

void DigHandler::operator()(KeyboardChordEvent const&) {
    handle_dig(map_manager.get_map(), entity, inventory, dx, dy);
}

ClickHandler::ClickHandler(MapManager& map_manager, Renderer& renderer):
    map_manager(map_manager),
    renderer(renderer)
{}

void ClickHandler::operator()(MouseEvent const& event) {
    long x = 0, y = 0;
    renderer.pixel_to_tile_coordinates(event.x, event.y, x, y);
    map_manager.get_map().click(x, y);
}

CreateAntHandler::CreateAntHandler(EntityManager& entity_manager,
    HardwareManager& hardware_manager,
    SoftwareManager& software_manager
):
    entity_manager(entity_manager),
    hardware_manager(hardware_manager),
    software_manager(software_manager)
{}


void CreateAntHandler::operator()(KeyboardEvent const&) {
    entity_manager.create_ant(hardware_manager, software_manager);
}

ReloadGameHandler::ReloadGameHandler(bool& is_reload_game):
    is_reload_game(is_reload_game)
{}

void ReloadGameHandler::operator()(KeyboardEvent const&) {
    is_reload_game = true;
}

DefaultMapTileRendererHandler::DefaultMapTileRendererHandler(Renderer& renderer):
    renderer(renderer)
{}

void DefaultMapTileRendererHandler::operator()(KeyboardEvent const&) {
    renderer.use_default_tile_rendering();
}

ScentMapTileRendererHandler::ScentMapTileRendererHandler(Renderer& renderer):
    renderer(renderer)
{}

void ScentMapTileRendererHandler::operator()(KeyboardEvent const& event) { 
    ulong scent_idx = event.type - KeyboardEventType::ONE_KEY_EVENT;
    renderer.use_scent_tile_rendering(scent_idx);
}

