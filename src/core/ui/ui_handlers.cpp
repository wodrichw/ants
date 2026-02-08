#include "ui/ui_handlers.hpp"

#include "app/engine_state.hpp"
#include "map/manager.hpp"

MoveHandler::MoveHandler(MapEntity& entity, MapManager& map_manager,
                         MapWorld& map_world, ulong& entity_depth,
                         bool move_only_on_current_depth, long dx, long dy,
                         std::function<bool()> is_input_blocked)
    : entity(entity),
      map_manager(map_manager),
      map_world(map_world),
      entity_depth(entity_depth),
      move_only_on_current_depth(move_only_on_current_depth),
      dx(dx),
      dy(dy),
      is_input_blocked(is_input_blocked) {}

void MoveHandler::operator()(KeyboardEvent const&) {
    if(is_input_blocked && is_input_blocked()) return;
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

SidebarToggleHandler::SidebarToggleHandler(EngineState& engine)
    : engine(engine) {}

void SidebarToggleHandler::operator()(KeyboardEvent const&) {
    engine.toggle_sidebar();
}

SidebarMouseToggleHandler::SidebarMouseToggleHandler(EngineState& engine,
                                                     Renderer& renderer)
    : engine(engine), renderer(renderer) {}

void SidebarMouseToggleHandler::operator()(MouseEvent const& event) {
    long tile_x = 0, tile_y = 0;
    renderer.pixel_to_tile_coordinates(event.x, event.y, tile_x, tile_y);

    long btn_x = 0, btn_y = 0, btn_w = 0, btn_h = 0;
    SidebarMenu::get_toggle_button_bounds(btn_x, btn_y, btn_w, btn_h);

    bool is_inside = tile_x >= btn_x && tile_x < (btn_x + btn_w) &&
                     tile_y >= btn_y && tile_y < (btn_y + btn_h);
    if(is_inside) {
        engine.toggle_sidebar();
    }
}

SidebarNavHandler::SidebarNavHandler(EngineState& engine, SidebarMenu& menu)
    : engine(engine), menu(menu) {}

void SidebarNavHandler::operator()(KeyboardEvent const& event) {
    if(!engine.box_manager.is_sidebar_expanded()) return;

    switch(event.type) {
        case UP_KEY_EVENT:
            menu.navigate_up();
            break;
        case DOWN_KEY_EVENT:
            menu.navigate_down();
            break;
        case RIGHT_KEY_EVENT:
        case RETURN_KEY_EVENT:
            menu.enter();
            break;
        case LEFT_KEY_EVENT:
        case BACKSPACE_KEY_EVENT:
            menu.back();
            break;
        default:
            return;
    }

    auto action = menu.take_pending_action();
    if(action.has_value()) {
        engine.handle_sidebar_action(action.value());
    }
}
