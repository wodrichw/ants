#pragma once

#include <unordered_map>
#include <vector>

#include "app/globals.hpp"
#include "entity/building.hpp"
#include "entity/entity_data.hpp"
#include "entity/map_section_data.hpp"
#include "proto/utils.pb.h"
#include "spdlog/spdlog.h"
#include "utils/math.hpp"

using ulong = unsigned long;

struct Tile {
    bool is_explored = false;  // has this tile already been seen by the player?
    bool in_fov = false;       // is the tile currently visible to the player?
    bool is_wall = true;
    MapEntity* entity = nullptr;
    Building* building = nullptr;
    Tile(bool is_explored = false, bool in_fov = false, bool is_wall = true)
        : is_explored(is_explored), in_fov(in_fov), is_wall(is_wall) {}
};

struct Chunk {
    long x = 0, y = 0;
    bool update_parity = true;
    std::vector<Tile> tiles;
    Chunk() = default;
    Chunk(long x, long y, bool update_parity)
        : x(x),
          y(y),
          update_parity(update_parity),
          tiles(globals::CHUNK_AREA) {}
    Chunk(Unpacker& p) {
        assert(globals::CHUNK_AREA == 64);

        ant_proto::Chunk msg;
        p >> msg;

        x = msg.x();
        y = msg.y();
        update_parity = msg.update_parity();
        tiles.reserve(globals::CHUNK_AREA);

        ulong is_explored = msg.is_explored(), in_fov = msg.in_fov(),
              is_wall = msg.is_wall();
        // SPDLOG_TRACE("Unpacking chunk - x: {} y: {} update: {} explored: {:x} fov: {:x} walls: {:x}", x, y, update_parity, is_explored, in_fov, is_wall);
        for(ulong i = 0; i < globals::CHUNK_AREA; ++i) {
            tiles.emplace_back((is_explored >> i) & 1UL, (in_fov >> i) & 1UL,
                               (is_wall >> i) & 1UL);
        }
    }
    Tile& operator[](long idx) {
        if(idx < 0 || idx >= globals::CHUNK_AREA)
            SPDLOG_ERROR("Invalid tile idx: {}", idx);

        return tiles[idx];
    }
    Tile const& operator[](long idx) const { return tiles[idx]; }

    friend Packer& operator<<(Packer& p, Chunk const& obj) {
        ant_proto::Chunk msg;
        msg.set_x(obj.x);
        msg.set_y(obj.y);
        msg.set_update_parity(obj.update_parity);

        ulong is_explored = 0, in_fov = 0, is_wall = 0;
        for(ulong i = 0; i < globals::CHUNK_AREA; ++i) {
            Tile const& tile = obj[i];
            is_explored |= ((tile.is_explored & 1UL) << i);
            in_fov |= ((tile.in_fov & 1UL) << i);
            is_wall |= ((tile.is_wall & 1UL) << i);
        }
        // SPDLOG_TRACE("Packing chunk - x: {} y: {} update: {} explored: {:x} fov: {:x} walls: {:x}", obj.x, obj.y, obj.update_parity, is_explored, in_fov, is_wall);

        msg.set_is_explored(is_explored);
        msg.set_in_fov(in_fov);
        msg.set_is_wall(is_wall);
        return p << msg;
    }
};

class Chunks {
    using ChunkMap = std::unordered_map<ulong, Chunk*>;

   public:
    class tile_iterator {
        using TileVector = std::vector<Tile>;

       public:
        tile_iterator(ChunkMap::iterator const& map_it)
            : map_it(map_it), tile_idx(0) {}

        tile_iterator& operator++() {
            ++tile_idx;
            // SPDLOG_TRACE("Incrementing tile iterator: {}", tile_idx);
            if(tile_idx >= get_tiles().size()) {
                ++map_it;
                tile_idx = 0;
            }
            return *this;
        }

        Tile& operator*() { return get_tiles()[tile_idx]; }
        bool operator!=(tile_iterator const& other) {
            return map_it != other.map_it || (tile_idx != other.tile_idx);
        }

       private:
        TileVector& get_tiles() { return map_it->second->tiles; }

        ChunkMap::iterator map_it;
        ulong tile_idx;
    };

    Chunks() = default;
    Chunks(Unpacker& p) {
        ant_proto::Chunks msg;
        p >> msg;

        ulong count = msg.count();
        SPDLOG_DEBUG("Unpacking chunks - count: {}", count);
        for(ulong i = 0; i < count; ++i) {
            ant_proto::Integer id_msg;
            p >> id_msg;

            Chunk* chunk = new Chunk(p);
            emplace(id_msg.value(), chunk);
        }
        SPDLOG_TRACE("Completed unpacking chunks");
    }

    tile_iterator begin_tile() { return tile_iterator(chunks.begin()); }
    tile_iterator end_tile() { return tile_iterator(chunks.end()); }

    ChunkMap::iterator begin() { return chunks.begin(); }
    ChunkMap::const_iterator begin() const { return chunks.begin(); }
    ChunkMap::iterator end() { return chunks.end(); }
    ChunkMap::const_iterator end() const { return chunks.end(); }

    ChunkMap::iterator find(ulong chunk_id) { return chunks.find(chunk_id); }
    ChunkMap::const_iterator find(ulong chunk_id) const {
        return chunks.find(chunk_id);
    }

    void erase(ChunkMap::iterator it) { chunks.erase(it); }
    Chunk& operator[](ulong chunk_id) { return *chunks[chunk_id]; }
    Chunk const& at(ulong chunk_id) const { return *chunks.at(chunk_id); }
    void emplace(ulong chunk_id, Chunk* chunk) {
        chunks.emplace(chunk_id, chunk);
    }

    friend Packer& operator<<(Packer& p, Chunks const& obj) {
        SPDLOG_DEBUG("Packing chunks - count: {}", obj.chunks.size());
        ant_proto::Chunks msg;
        msg.set_count(obj.chunks.size());
        p << msg;

        for(auto const& [id, chunk] : obj.chunks) {
            ant_proto::Integer id_msg;
            id_msg.set_value(id);
            p << id_msg << (*chunk);
        }
        SPDLOG_TRACE("Completed packing chunks");
        return p;
    }

   private:
    ChunkMap chunks;
};

class Map {
   public:
    bool needs_update = true;
    bool chunk_update_parity = false;

    Map(Rect const& border, bool is_walls_enabled)
        : is_walls_enabled(is_walls_enabled) {
        SPDLOG_INFO("Creating map with border: ({}, {}) - {}x{}", border.x1,
                    border.y1, border.w, border.h);
        add_missing_chunks(border);
    }

    Map(Unpacker& p) : chunks(p) {
        ant_proto::Map msg;
        p >> msg;

        needs_update = msg.needs_update();
        chunk_update_parity = msg.chunk_update_parity();
        is_walls_enabled = msg.is_walls_enabled();
        SPDLOG_DEBUG("Unpacking map - needs update: {} chunk update: {} is walls: {}",
            needs_update ? "YES" : "NO",
            chunk_update_parity ? "ON" : "OFF",
            is_walls_enabled ? "ENABLED" : "DISABLED"
        );
    }

    void load_section(MapSectionData const& section_data) {
        SPDLOG_INFO("Loading section data into map: ({}, {}) - {}x{}",
                    section_data.border.x1, section_data.border.y1,
                    section_data.border.w, section_data.border.h);
        add_missing_chunks(section_data.border);

        long origin_x = section_data.border.x1,
             origin_y = section_data.border.y1;
        for(auto const& room : section_data.rooms) {
            dig(origin_x + room.x1, origin_y + room.y1, origin_x + room.x2,
                origin_y + room.y2);
        }

        for(auto const& corridor : section_data.corridors) {
            dig(origin_x + corridor.x1, origin_y + corridor.y1,
                origin_x + corridor.x2, origin_y + corridor.y2);
        }
    }

    void dig(long x1, long y1, long x2, long y2) {
        if(x2 < x1) std::swap(x1, x2);
        if(y2 < y1) std::swap(y1, y2);
        SPDLOG_TRACE("Digging from ({}, {}) to ({}, {})", x1, y1, x2, y2);

        // SPDLOG_TRACE("Setting properties for tiles");
        for(int tilex = x1 - 1; tilex <= x2 + 1; tilex++) {
            bool is_horizontal_wall = tilex < x1 || tilex > x2;

            for(int tiley = y1 - 1; tiley <= y2 + 1; tiley++) {
                bool is_vertical_wall = tiley < y1 || tiley > y2;

                Tile& tile = get_tile(tilex, tiley);
                if(!tile.is_wall) continue;

                tile.is_wall = is_horizontal_wall || is_vertical_wall;
                // SPDLOG_TRACE("Setting tile at ({}, {}) - wall: {}, floor:
                // {}", tilex, tiley, tile.is_wall);
            }
        }
        SPDLOG_TRACE("Digging complete");
    }

    bool can_place(long x, long y) const {
        Tile const& tile = get_tile_const(x, y);
        if(tile.is_wall) return false;
        if(tile.entity != nullptr) {
            tile.entity->request_move();
            return tile.entity == nullptr;
        }
        return true;
    }

    void add_entity(MapEntity& entity) {
        EntityData& data = entity.get_data();
        set_entity(data.x, data.y, &entity);
    }

    void remove_entity(MapEntity& entity) {
        SPDLOG_TRACE("Removing entity");
        EntityData& data = entity.get_data();
        set_entity(data.x, data.y, nullptr);
    }

    bool move_entity(MapEntity& entity, long dx, long dy) {
        SPDLOG_DEBUG("Moving entity by - dx: {} dy: {}", dx, dy);
        EntityData& data = entity.get_data();
        long x = data.x, y = data.y;

        long new_x = x + dx, new_y = y + dy;
        SPDLOG_TRACE("Moving entity - new x: {} new y: {}", new_x, new_y);

        // remove entity from map so another entity can take its place if needed
        remove_entity(entity);
        if(is_walls_enabled && !can_place(new_x, new_y)) {
            SPDLOG_TRACE("Cannot move entity to ({}, {})", new_x, new_y);

            // unable to move the obstacle so move the entity back
            add_entity(entity);
            return false;
        }

        // notify that the entity was successfully removed
        notify_all_removed_entity(x, y);

        data.x = new_x;
        data.y = new_y;
        add_entity(entity);

        // notify that the entity was successfully moved
        notify_all_moved_entity(new_x, new_y, entity);

        SPDLOG_TRACE("Calling entity move callback");
        entity.move_callback(x, y, new_x, new_y);
        SPDLOG_TRACE("Successfully moved the entity");
        return true;
    }

    bool dig(MapEntity& entity, long dx, long dy) {
        SPDLOG_DEBUG("Entity is digging - dx: {} dy: {}", dx, dy);
        EntityData& data = entity.get_data();
        long x = data.x, y = data.y;

        long new_x = x + dx, new_y = y + dy;
        SPDLOG_TRACE("Entity digging - new x: {} new y: {}", new_x, new_y);

        Tile& tile = get_tile(new_x, new_y);
        if (!tile.is_wall) {
            SPDLOG_TRACE("Failed to dig at non-wall position");
            return false;
        }        

        tile.is_wall = false;
        SPDLOG_TRACE("Entity successfully dug at - x: {} y: {}", new_x, new_y);
        return true;
    }

    void add_building(Building& building) {
        for(long x = building.border.x1; x <= building.border.x2; ++x) {
            for(long y = building.border.y1; y <= building.border.y2; ++y) {
                get_tile(x, y).building = &building;
            }
        }
    }

    Building* get_building(MapEntity& entity) {
        EntityData& data = entity.get_data();
        return get_tile(data.x, data.y).building;
    }

    void create_chunk(long x, long y) {
        ulong const chunk_id = get_chunk_idx(x, y);
        auto it = chunks.find(chunk_id);
        if(it != chunks.end()) {
            Chunk& chunk = *it->second;
            chunk.update_parity = chunk_update_parity;
            return;
        }

        long aligned_x =
                 div_floor(x, globals::CHUNK_LENGTH) * globals::CHUNK_LENGTH,
             aligned_y =
                 div_floor(y, globals::CHUNK_LENGTH) * globals::CHUNK_LENGTH;

        SPDLOG_DEBUG("Adding chunk ({}, {}) - id: {}", x, y, chunk_id);
        chunks.emplace(chunk_id,
                       new Chunk(aligned_x, aligned_y, chunk_update_parity));
    }

    void add_missing_chunks(Rect const& rect) {
        long buffer = 1;
        long x1 = (div_floor(rect.x1, globals::CHUNK_LENGTH) - buffer) *
                  globals::CHUNK_LENGTH,
             y1 = (div_floor(rect.y1, globals::CHUNK_LENGTH) - buffer) *
                  globals::CHUNK_LENGTH;
        long x2 = rect.x2 + buffer * globals::CHUNK_LENGTH,
             y2 = rect.y2 + buffer * globals::CHUNK_LENGTH;

        SPDLOG_DEBUG("Loading missing chunks: ({}, {}) - ({}, {})", x1, y1, x2,
                     y2);
        for(long x = x1; x <= x2; x += globals::CHUNK_LENGTH) {
            for(long y = y1; y <= y2; y += globals::CHUNK_LENGTH) {
                create_chunk(x, y);
            }
        }
        SPDLOG_TRACE("Finished loading missing chunks");
    }

    void remove_unused_chunks() {
        SPDLOG_TRACE("Removing unused chunks");
        for(auto it = chunks.begin(); it != chunks.end();) {
            Chunk& chunk = *it->second;
            if(chunk.update_parity == chunk_update_parity) {
                ++it;
                continue;
            }

            chunks.erase(it++);
        }
        SPDLOG_TRACE("Finished removing unused chunks");
    }

    void update_chunks(Rect const& rect) {
        SPDLOG_DEBUG("Updating chunks: ({}, {}) - ({}, {})", rect.x1, rect.y1,
                     rect.x2, rect.y2);
        chunk_update_parity = !chunk_update_parity;
        add_missing_chunks(rect);
        // remove_unused_chunks();
        SPDLOG_TRACE("Finished updating chunks");
    }

    void reset_fov() {
        for(auto it = chunks.begin_tile(); it != chunks.end_tile(); ++it) {
            Tile& tile = *it;
            tile.in_fov = false;
        }
    }

    void reset_tile(long x, long y) {
        Tile& tile = get_tile(x, y);
        tile.in_fov = false;
    }

    void explore(long x, long y) {
        Tile& tile = get_tile(x, y);
        tile.is_explored = true;
        tile.in_fov = true;
    }

    bool in_fov(long x, long y) const {
        // SPDLOG_TRACE("Checking if tile at ({}, {}) is in fov", x, y);
        return get_tile_const(x, y).in_fov;
    }

    bool is_explored(long x, long y) const {
        // SPDLOG_TRACE("Checking if tile at ({}, {}) is explored", x, y);
        return get_tile_const(x, y).is_explored;
    }

    bool is_wall(long x, long y) const { return get_tile_const(x, y).is_wall; }

    bool click(long x, long y) {
        MapEntity* entity = get_tile(x, y).entity;
        if(entity == nullptr) return false;
        entity->click_callback(x, y);
        return true;
    }

    ulong get_chunk_idx(long x, long y) const {
        long chunk_x = div_floor(x, globals::CHUNK_LENGTH);
        long chunk_y = div_floor(y, globals::CHUNK_LENGTH);
        long chunk_depth = std::max(std::abs(chunk_x), std::abs(chunk_y));

        long offset =
            chunk_y == -chunk_depth || chunk_x == -chunk_depth ? 0 : 1;
        long depth_sqrt_idx = 2 * (chunk_depth - offset) + 1;
        long depth_idx = depth_sqrt_idx * depth_sqrt_idx + offset - 1;
        long chunk_idx =
            depth_idx +
            (offset * 2 - 1) * (2 * chunk_depth - chunk_x + chunk_y - offset);
        // SPDLOG_TRACE("Chunk index for tile ({}, {}) is {}", x, y, chunk_idx);
        return chunk_idx;
    }

    friend Packer& operator<<(Packer& p, Map const& obj) {
        SPDLOG_DEBUG("Packing map - needs update: {} chunk update: {} is walls: {}",
            obj.needs_update ? "YES" : "NO",
            obj.chunk_update_parity ? "ON" : "OFF",
            obj.is_walls_enabled ? "ENABLED" : "DISABLED"
        );
        ant_proto::Map msg;
        msg.set_needs_update(obj.needs_update);
        msg.set_chunk_update_parity(obj.chunk_update_parity);
        msg.set_is_walls_enabled(obj.is_walls_enabled);
        return p << obj.chunks << msg;
    }

   private:
    Chunk& get_chunk(long x, long y) {
        create_chunk(x, y);
        return chunks[get_chunk_idx(x, y)];
    }

    Chunk const& get_chunk_const(long x, long y) const {
        return chunks.at(get_chunk_idx(x, y));
    }

    long get_local_idx(long chunk_x, long chunk_y, long x, long y) const {
        long local_idx = (x - chunk_x) + (y - chunk_y) * globals::CHUNK_LENGTH;
        // SPDLOG_TRACE("Local index for tile ({}, {}) is {}", x, y, local_idx);
        return local_idx;
    }

    Tile& get_tile(long x, long y) {
        Chunk& chunk = get_chunk(x, y);
        return chunk[get_local_idx(chunk.x, chunk.y, x, y)];
    }
    Tile const& get_tile_const(long x, long y) const {
        Chunk const& chunk = get_chunk_const(x, y);
        return chunk[get_local_idx(chunk.x, chunk.y, x, y)];
    }

    uchar flip_direction_bits(uchar bits) {
        //DLUR -> URDL
        return ((bits >> 2) | (bits << 2)) & 0b1111;
    }

    void notify_removed_entity(long x, long y, uchar bits) {
        MapEntity* entity = get_tile(x, y).entity;
        if (entity == nullptr)  return;
        entity->handle_empty_space(bits);
    }

    void notify_moved_entity(MapEntity& source, long x, long y, uchar bits) {
        uchar source_bits = flip_direction_bits(bits);
        if (can_place(x, y)) {
            source.handle_empty_space(source_bits);
            return;
        }
        source.handle_full_space(source_bits);
    
        MapEntity* entity = get_tile(x, y).entity;
        if (entity == nullptr) return;
        entity->handle_full_space(bits);
    }

    void set_entity(long x, long y, MapEntity* entity) {
        SPDLOG_DEBUG("Setting entity at ({}, {})", x, y);
        get_tile(x, y).entity = entity;
        needs_update = true;
    }

    void notify_all_removed_entity(long x, long y) {
        notify_removed_entity(x - 1, y, 0b0001); // right
        notify_removed_entity(x, y + 1, 0b0010); // up
        notify_removed_entity(x + 1, y, 0b0100); // left
        notify_removed_entity(x, y - 1, 0b1000); // down
    }
    
    void notify_all_moved_entity(long x, long y, MapEntity& entity) {
        notify_moved_entity(entity, x - 1, y, 0b0001); // right
        notify_moved_entity(entity, x, y + 1, 0b0010); // up
        notify_moved_entity(entity, x + 1, y, 0b0100); // left
        notify_moved_entity(entity, x, y - 1, 0b1000);  // down
    }

    Chunks chunks;
    bool is_walls_enabled;
};
