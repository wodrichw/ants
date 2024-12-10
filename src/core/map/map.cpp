#include "map/map.hpp"
#include "app/globals.hpp"
#include "spdlog/spdlog.h"
#include "utils/math.hpp"

using ulong = unsigned long;

Chunk::Chunk(long x, long y, bool update_parity) :
    x(x),
    y(y),
    update_parity(update_parity),
    tiles(globals::CHUNK_AREA) 
{}


Chunk::Chunk(const ant_proto::Chunk& msg):
    x(msg.x()),
    y(msg.y()),
    update_parity(msg.update_parity()),
    tiles()
{
    tiles.reserve(globals::CHUNK_AREA);
    ulong is_explored = msg.is_explored(), in_fov = msg.in_fov(),
          is_wall = msg.is_wall();
    for(ulong i = 0; i < globals::CHUNK_AREA; ++i) {
        tiles.emplace_back((is_explored >> i) & 1UL, (in_fov >> i) & 1UL,
                           (is_wall >> i) & 1UL);
    }
}


Tile& Chunk::operator[](long idx) {
    if(idx < 0 || idx >= globals::CHUNK_AREA)
        SPDLOG_ERROR("Invalid tile idx: {}", idx);

    return tiles[idx];
}


Tile const& Chunk::operator[](long idx) const {
    return tiles[idx];
}


ant_proto::Chunk Chunk::get_proto() {
    ant_proto::Chunk msg;
    msg.set_x(x);
    msg.set_y(y);
    msg.set_update_parity(update_parity);

    ulong is_explored = 0, in_fov = 0, is_wall = 0;
    for(ulong i = 0; i < globals::CHUNK_AREA; ++i) {
        Tile const& tile = (*this)[i];
        is_explored |= ((tile.is_explored & 1UL) << i);
        in_fov |= ((tile.in_fov & 1UL) << i);
        is_wall |= ((tile.is_wall & 1UL) << i);
    }
    // SPDLOG_TRACE("Packing chunk - x: {} y: {} update: {} explored: {:x} fov: {:x} walls: {:x}", obj.x, obj.y, obj.update_parity, is_explored, in_fov, is_wall);

    msg.set_is_explored(is_explored);
    msg.set_in_fov(in_fov);
    msg.set_is_wall(is_wall);

    return msg;
}


Chunks::tile_iterator& Chunks::tile_iterator::operator++() {
    ++tile_idx;
    // SPDLOG_TRACE("Incrementing tile iterator: {}", tile_idx);
    if(tile_idx >= get_tiles().size()) {
        ++map_it;
        tile_idx = 0;
    }
    return *this;
}


Tile& Chunks::tile_iterator::operator*() {
    return get_tiles()[tile_idx];
}


bool Chunks::tile_iterator::operator!=(Chunks::tile_iterator const& other) {
    return map_it != other.map_it || (tile_idx != other.tile_idx);
}


Chunks::tile_iterator::TileVector& Chunks::tile_iterator::get_tiles() {
    return map_it->second->tiles;
}


Chunks::Chunks(const ant_proto::Chunks& msg) {
    for(const auto& chunk_key_val: msg.chunk_key_vals()) {
        SPDLOG_TRACE("unpacking chunk {}", chunk_key_val.key());
        Chunk* chunk = new Chunk(chunk_key_val.val());
        emplace(chunk_key_val.key(), chunk);
    }
}


long Chunks::align(long pos) const {
     return div_floor(pos, globals::CHUNK_LENGTH) * globals::CHUNK_LENGTH;
}


ulong Chunks::get_chunk_id(long x, long y) const {
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


std::vector<ChunkMarker> Chunks::get_chunk_markers(const Rect& rect) const {
    std::vector<ChunkMarker> c_markers;

    for(long x = rect.x1; x <= rect.x2 + globals::CHUNK_LENGTH; x += globals::CHUNK_LENGTH) {
        for(long y = rect.y1; y <= rect.y2 + globals::CHUNK_LENGTH; y += globals::CHUNK_LENGTH) {
            ulong id = get_chunk_id(x, y);
            auto it = std::lower_bound(c_markers.begin(), c_markers.end(), id);
            if( it != c_markers.end() && it->id == id ) continue; // already in c_markers
            c_markers.insert(it, {align(x), align(y), id});
        }
    }
    return c_markers;
}


ant_proto::Chunks Chunks::get_proto() const {
    ant_proto::Chunks msg;
    for( const auto& chunk: chunks ) {
        ant_proto::ChunkKeyVal kv_msg;
        kv_msg.set_key(chunk.first);
        *kv_msg.mutable_val() = chunk.second->get_proto();
        *msg.add_chunk_key_vals() = kv_msg;
    }
    return msg;
}


Map::Map(bool is_walls_enabled) :
    is_walls_enabled(is_walls_enabled) 
{ }


Map::Map(Rect const& border, bool is_walls_enabled)
    : is_walls_enabled(is_walls_enabled) {
    SPDLOG_INFO("Creating map with border: ({}, {}) - {}x{}", border.x1,
                border.y1, border.w, border.h);
    add_missing_chunks(border);
}


Map::Map(const ant_proto::Map& msg, bool is_walls_enabled):
    needs_update(msg.needs_update()),
    chunk_update_parity(msg.chunk_update_parity()),
    chunks(msg.chunks()),
    is_walls_enabled(is_walls_enabled)
{
    SPDLOG_DEBUG("Unpacking map - needs update: {} chunk update: {} is walls: {}",
        needs_update ? "YES" : "NO",
        chunk_update_parity ? "ON" : "OFF",
        is_walls_enabled ? "ENABLED" : "DISABLED"
    );
}


void Map::load_section(MapSectionData const& section_data) {
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

    for(const auto& cm: chunks.get_chunk_markers(section_data.border) )
        chunks.find(cm.id)->second->section_loaded = true;
}

void Map::dig(long x1, long y1, long x2, long y2) {
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


bool Map::can_place(long x, long y) const {
    Tile const& tile = get_tile_const(x, y);
    if(tile.is_wall) return false;
    if(tile.entity != nullptr) {
        tile.entity->request_move();
        return tile.entity == nullptr;
    }
    return true;
}


void Map::add_entity_wo_events(MapEntity& entity) {
    EntityData& data = entity.get_data();
    set_entity(data.x, data.y, &entity);
}


void Map::add_entity(MapEntity& entity) {
    add_entity_wo_events(entity);
    
    // notify that the entity was successfully moved
    auto entity_data = entity.get_data();
    notify_all_moved_entity(entity_data.x, entity_data.y, entity);
}


void Map::remove_entity(MapEntity& entity) {
    SPDLOG_TRACE("Removing entity");
    EntityData& data = entity.get_data();
    set_entity(data.x, data.y, nullptr);
}


bool Map::move_entity(MapEntity& entity, long dx, long dy) {
    SPDLOG_DEBUG("Moving entity by - dx: {} dy: {}", dx, dy);
    EntityData& data = entity.get_data();
    long x = data.x, y = data.y;

    long new_x = x + dx, new_y = y + dy;

    SPDLOG_TRACE("Calling entity move callback");
    ulong right_scents = get_tile(new_x + 1, new_y).scents;
    ulong up_scents = get_tile(new_x, new_y - 1).scents;
    ulong left_scents = get_tile(new_x - 1, new_y).scents;
    ulong down_scents = get_tile(new_x, new_y + 1).scents;

    entity.move_callback({ x, y, new_x, new_y,
        {right_scents, up_scents, left_scents, down_scents}});

    SPDLOG_TRACE("Moving entity - new x: {} new y: {}", new_x, new_y);

    // remove entity from map so another entity can take its place if needed
    remove_entity(entity);
    if(is_walls_enabled && !can_place(new_x, new_y)) {
        SPDLOG_TRACE("Cannot move entity to ({}, {})", new_x, new_y);

        // unable to move the obstacle so move the entity back
        add_entity_wo_events(entity);
        return false;
    }

    // notify that the entity was successfully removed
    notify_all_removed_entity(x, y);

    data.x = new_x;
    data.y = new_y;
    add_entity(entity);

    // This will likely need to be changed once more sophisticated map generation is put in place
    add_missing_chunks(Rect(new_x, new_y, 1, 1));

    // notify that the entity was successfully moved
    notify_all_moved_entity(new_x, new_y, entity);

    SPDLOG_TRACE("Successfully moved the entity");
    return true;
}


bool Map::dig(MapEntity& entity, long dx, long dy) {
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


void Map::add_building(Building& building) {
    for(long x = building.border.x1; x <= building.border.x2; ++x) {
        for(long y = building.border.y1; y <= building.border.y2; ++y) {
            get_tile(x, y).building = &building;
        }
    }
}


Building* Map::get_building(MapEntity& entity) {
    EntityData& data = entity.get_data();
    return get_tile(data.x, data.y).building;
}


void Map::create_chunk(long x, long y) {
    ulong const chunk_id = chunks.get_chunk_id(x, y);
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
    chunks.emplace(chunk_id, new Chunk(aligned_x, aligned_y, chunk_update_parity));
}


void Map::add_missing_chunks(Rect const& rect) {
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


std::vector<ChunkMarker> Map::get_chunk_markers(const Rect& rect) const {
    return chunks.get_chunk_markers(rect);
}


void Map::remove_unused_chunks() {
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


void Map::update_chunks(Rect const& rect) {
    SPDLOG_DEBUG("Updating chunks: ({}, {}) - ({}, {})", rect.x1, rect.y1,
                 rect.x2, rect.y2);
    chunk_update_parity = !chunk_update_parity;
    add_missing_chunks(rect);
    // remove_unused_chunks();
    SPDLOG_TRACE("Finished updating chunks");
}


void Map::reset_fov() {
    for(auto it = chunks.begin_tile(); it != chunks.end_tile(); ++it) {
        Tile& tile = *it;
        tile.in_fov = false;
    }
}


void Map::reset_tile(long x, long y) {
    Tile& tile = get_tile(x, y);
    tile.in_fov = false;
}


void Map::explore(long x, long y) {
    Tile& tile = get_tile(x, y);
    tile.is_explored = true;
    tile.in_fov = true;
}


bool Map::chunk_built(const ChunkMarker& cm) const {
    auto c_itr = chunks.find(cm.id);
    if( c_itr == chunks.end() ) return false;
    return c_itr->second->section_loaded;
}

bool Map::chunk_built(long x, long y) const {
    ulong id = chunks.get_chunk_id(x,y);
    auto c_itr = chunks.find(id);
    if( c_itr == chunks.end() ) return false;
    return c_itr->second->section_loaded;
}


bool Map::in_fov(long x, long y) const {
    // SPDLOG_TRACE("Checking if tile at ({}, {}) is in fov", x, y);

    return get_tile_const(x, y).in_fov;
}


bool Map::is_explored(long x, long y) const {
    // SPDLOG_TRACE("Checking if tile at ({}, {}) is explored", x, y);
    return get_tile_const(x, y).is_explored;
}


bool Map::is_wall(long x, long y) const {
    return get_tile_const(x, y).is_wall;
}


bool Map::click(long x, long y) {
    MapEntity* entity = get_tile(x, y).entity;
    if(entity == nullptr) return false;
    entity->click_callback(x, y);
    return true;
}


ulong& Map::get_tile_scents(MapEntity& entity) {
    EntityData& data = entity.get_data();
    return get_tile(data.x, data.y).scents;
}


ulong Map::get_tile_scents_by_coord(long x, long y) const {
    return get_tile_const(x, y).scents;
}


ant_proto::Map Map::get_proto() const {
    ant_proto::Map msg;
    msg.set_needs_update(needs_update);
    msg.set_chunk_update_parity(chunk_update_parity);
    *msg.mutable_chunks() = chunks.get_proto();
    return msg;
}


Chunk& Map::get_chunk(long x, long y) {
    create_chunk(x, y);
    return chunks[{x, y}];
}


Chunk const& Map::get_chunk_const(long x, long y) const {
    return chunks.at(chunks.get_chunk_id(x, y));
}


long Map::get_local_idx(long chunk_x, long chunk_y, long x, long y) const {
    long local_idx = (x - chunk_x) + (y - chunk_y) * globals::CHUNK_LENGTH;
    // SPDLOG_TRACE("Local index for tile ({}, {}) is {}", x, y, local_idx);
    return local_idx;
}


Tile& Map::get_tile(long x, long y) {
    Chunk& chunk = get_chunk(x, y);
    return chunk[get_local_idx(chunk.x, chunk.y, x, y)];
}


Tile const& Map::get_tile_const(long x, long y) const {
    Chunk const& chunk = get_chunk_const(x, y);
    return chunk[get_local_idx(chunk.x, chunk.y, x, y)];
}


uchar Map::flip_direction_bits(uchar bits) {
    //DLUR -> URDL
    return ((bits >> 2) | (bits << 2)) & 0b1111;
}


void Map::notify_removed_entity(long x, long y, uchar bits) {
    MapEntity* entity = get_tile(x, y).entity;
    if (entity == nullptr)  return;
    entity->handle_empty_space(bits);
}


void Map::notify_moved_entity(MapEntity& source, long x, long y, uchar bits) {
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


void Map::set_entity(long x, long y, MapEntity* entity) {
    SPDLOG_DEBUG("Setting entity at ({}, {})", x, y);
    get_tile(x, y).entity = entity;
    needs_update = true;
}


void Map::notify_all_removed_entity(long x, long y) {
    notify_removed_entity(x - 1, y, 0b0001); // right
    notify_removed_entity(x, y + 1, 0b0010); // up
    notify_removed_entity(x + 1, y, 0b0100); // left
    notify_removed_entity(x, y - 1, 0b1000); // down
}


void Map::notify_all_moved_entity(long x, long y, MapEntity& entity) {
    notify_moved_entity(entity, x - 1, y, 0b0001); // right
    notify_moved_entity(entity, x, y + 1, 0b0010); // up
    notify_moved_entity(entity, x + 1, y, 0b0100); // left
    notify_moved_entity(entity, x, y - 1, 0b1000);  // down
}

