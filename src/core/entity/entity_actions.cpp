#include "map/map.hpp"
#include "entity/entity_data.hpp"
#include "entity/inventory.hpp"
#include "entity/entity_actions.hpp"
#include "spdlog/spdlog.h"

void handle_dig(Map& map, MapEntity& entity, Inventory& inventory, long dx, long dy) {
    ulong dirt_quantity = 0;
    if(inventory.max_space_for_item(DIRT) < dirt_quantity) return;
    if(!map.dig(entity, dx, dy)) return;


    inventory.add(DIRT, dirt_quantity);
    if (dirt_quantity > 0) {
        SPDLOG_ERROR("Failed to add dirt to inventory");
        return;
    }
}
