#pragma once

#include <vector>

#include "app/arg_parse.hpp"
#include "entity/map_builder.hpp"
#include "entity/building.hpp"
#include "entity/map_entity.hpp"
#include "entity/map_reader.hpp"

struct Tile {
    MapEntity* entity = nullptr;
    Building* building = nullptr;
    bool is_explored = false;  // has the player already seen this tile ?
    bool in_fov = false;
};

class Map {
public:
    int width, height;
    bool need_update_fov;

    Map(MapBuilder& builder, ProjectArguments &config):
        width(builder.width),
        height(builder.height),
        need_update_fov(true),
        tiles_list(width * height),
        builder(builder)
    {
        if (config.default_map_file_path.empty()) {
            RandomMapReader()(builder);
            RoomRect* first_room = builder.get_first_room();
            if( !first_room ) return;
            // add_building(Building &building)
        } else {
            FileMapReader(config.default_map_file_path)(builder);
        }
    }

    bool can_place(long x, long y) {
        return get_tile(x, y).entity == nullptr && builder.can_place(x, y);
    }

    void add_entity(MapEntity& entity) {
        MapData& data = entity.get_data();
        set_entity(data.x, data.y, &entity);
    }

    void remove_entity(MapEntity& entity) {
        MapData& data = entity.get_data();
        set_entity(data.x, data.y, nullptr);
    }

    bool move_entity(MapEntity& entity, long dx, long dy) {
        MapData& data = entity.get_data();
        long x = data.x, y = data.y;

        long new_x = x + dx, new_y = y + dy;
        if (!can_place(new_x, new_y)) return false;
    
        set_entity(new_x, new_y, &entity);
        remove_entity(entity);
        data.x = new_x;
        data.y = new_y;

        entity.move_callback(x, y, new_x, new_y);
        return true;
    }

    void add_building(Building& building) {
        for (long x = building.x; x < building.x + building.w; ++x) {
            for (long y = building.y; y < building.y + building.h; ++y) {
                get_tile(x, y).building = &building;
            }
        }
    }

    Building* get_building(MapEntity& entity) {
        MapData& data = entity.get_data();
        return get_tile(data.x, data.y).building;
    }

    void compute_fov(long x, long y, long radius) {
        builder.compute_fov(x, y, radius);
    }

    void reset_fov() {
        for(long x = 0; x < width; x++) {
            for(long y = 0; y < height; y++) {
                Tile& tile = get_tile(x, y);
                tile.in_fov = false;
            }
        }
    }

    void update_fov() {
        for (long x = 0; x < builder.width; ++x) {
            for (long y = 0; y < builder.height; ++y) {
                if (!builder.in_fov(x, y)) continue;
                Tile& tile = get_tile(x, y);
                tile.in_fov = tile.is_explored = true;
            }
        }
    }

    bool in_fov(long x, long y) const {
        // SPDLOG_TRACE("Checking if tile at ({}, {}) is in fov", x, y); 
        return get_tile(x, y).in_fov;
    }

    bool is_explored(long x, long y) const {
        // SPDLOG_TRACE("Checking if tile at ({}, {}) is explored", x, y); 
        return get_tile(x, y).is_explored;
    }

    bool is_wall(long x, long y) const {
        return builder.is_wall(x, y);
    }

    bool click(long x, long y) {
        MapEntity* entity = get_tile(x, y).entity;
        if (entity == nullptr) return false;
        entity->click_callback(x, y);
        return true;
    }

private:
    Tile& get_tile(long x, long y) { return tiles_list[x + y * builder.width]; }
    Tile const& get_tile(long x, long y) const { return tiles_list[x + y * builder.width]; }
    void set_entity(long x, long y, MapEntity* entity) {
        get_tile(x, y).entity = entity;
        need_update_fov = true;
    }

    std::vector<Tile> tiles_list;
    MapBuilder& builder;
};

