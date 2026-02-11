#pragma once

#include <unordered_map>
#include <vector>

#include "app/globals.hpp"
#include "utils/types.hpp"
#include "entity/building.hpp"
#include "entity/entity_data.hpp"
#include "map.pb.h"
#include "map/section_data.hpp"

struct Tile {
    bool is_explored = false;  // has this tile already been seen by the player?
    bool in_fov = false;       // is the tile currently visible to the player?
    bool is_wall = true;
    MapEntity* entity = nullptr;
    Building* building = nullptr;
    ulong scents = 0;
    Tile(bool is_explored = false, bool in_fov = false, bool is_wall = true)
        : is_explored(is_explored), in_fov(in_fov), is_wall(is_wall) {}
};

struct ChunkMarker {
    long x;
    long y;
    ulong id;
    bool operator==(ulong rhs_id) const { return id == rhs_id; }
    bool operator==(const ChunkMarker& rhs) const { return id == rhs.id; }
    bool operator<(ulong rhs_id) const { return id < rhs_id; }
    bool operator<(const ChunkMarker& rhs) const { return id < rhs.id; }
};

struct Chunk {
    long x = 0, y = 0;
    bool update_parity = true;
    bool section_loaded = false;
    std::vector<Tile> tiles;

    Chunk() = default;
    Chunk(long x, long y, bool update_parity);
    Chunk(const ant_proto::Chunk& msg);

    Tile& operator[](long idx);
    Tile const& operator[](long idx) const;

    ant_proto::Chunk get_proto();
};

class Chunks {
    using ChunkMap = std::unordered_map<ulong, Chunk*>;

   public:
    class tile_iterator {
        using TileVector = std::vector<Tile>;

       public:
        tile_iterator(ChunkMap::iterator const& map_it)
            : map_it(map_it), tile_idx(0) {}

        tile_iterator& operator++();

        Tile& operator*();
        bool operator!=(tile_iterator const& other);

       private:
        TileVector& get_tiles();

        ChunkMap::iterator map_it;
        ulong tile_idx;
    };

    Chunks() = default;

    Chunks(const ant_proto::Chunks& msg);

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

    void create_chunk(const ChunkMarker& cm);
    std::vector<ChunkMarker> get_chunk_markers(const Rect& rect) const;

    void erase(ChunkMap::iterator it) { chunks.erase(it); }
    long align(long pos) const;  // takes a tile pos and aligns it to chunk
    ulong get_chunk_id(long x, long y) const;
    Chunk& operator[](ulong chunk_id) { return *chunks[chunk_id]; }
    Chunk& operator[](std::pair<long, long> p) {
        return *chunks[get_chunk_id(p.first, p.second)];
    }
    Chunk const& at(ulong chunk_id) const { return *chunks.at(chunk_id); }
    void emplace(ulong chunk_id, Chunk* chunk) {
        chunks.emplace(chunk_id, chunk);
    }

    ant_proto::Chunks get_proto() const;

   private:
    ChunkMap chunks;
};

class Map {
    using f_xy_t = std::function<void(long, long)>;
    f_xy_t generate_chunk_callback;

   public:
    bool needs_update = true;
    bool chunk_update_parity = false;

    Map(bool is_walls_enabled, f_xy_t pre_chunk_generation_callback);
    Map(Rect const& border, bool is_walls_enabled,
        f_xy_t pre_chunk_generation_callback);
    Map(const ant_proto::Map& msg, bool is_walls_enabled,
        f_xy_t pre_chunk_generation_callback);

    void load_section(MapSectionData const& section_data);
    void dig(long x1, long y1, long x2, long y2);
    bool can_place(long x, long y);
    void add_entity_wo_events(MapEntity& entity);
    void add_entity(MapEntity& entity);
    void remove_entity(MapEntity& entity);
    bool move_entity(MapEntity& entity, long dx, long dy);
    bool dig(MapEntity& entity, long dx, long dy);
    void add_building(Building& building);
    Building* get_building(MapEntity& entity);
    void create_chunk(long x, long y);
    std::vector<ChunkMarker> get_chunk_markers(const Rect& rect) const;
    void remove_unused_chunks();
    void update_chunks(Rect const& rect);
    void reset_fov();
    void reset_tile(long x, long y);
    void explore(long x, long y);
    bool chunk_built(const ChunkMarker& cm) const;
    bool chunk_built(long x, long y) const;
    bool in_fov(long x, long y);
    bool is_explored(long x, long y);
    bool is_wall(long x, long y);
    bool click(long x, long y);
    ulong& get_tile_scents(MapEntity& entity);
    ulong get_tile_scents_by_coord(long x, long y);
    ant_proto::Map get_proto() const;

   private:
    Chunk& get_chunk(long x, long y);
    Chunk const& get_chunk_const(long x, long y) const;
    long get_local_idx(long chunk_x, long chunk_y, long x, long y) const;
    Tile& get_tile(long x, long y);
    uchar flip_direction_bits(uchar bits);
    void notify_removed_entity(long x, long y, uchar bits);
    void notify_moved_entity(MapEntity& source, long x, long y, uchar bits);
    void set_entity(long x, long y, MapEntity* entity);
    void notify_all_removed_entity(long x, long y);
    void notify_all_moved_entity(long x, long y, MapEntity& entity);

    Chunks chunks;
    bool is_walls_enabled;
};
