#include "map.hpp"

#include <libtcod/console_drawing.h>
#include <libtcod.hpp>
#include <libtcod/bsp.hpp>
#include <libtcod/color.hpp>

#include <vector>
#include <fstream>
#include <sstream>

#include "ant.hpp"
#include "building.hpp"
#include "arg_parse.hpp"
#include "spdlog/spdlog.h"

using ulong = unsigned long;

class BspListener : public ITCODBspCallback {
   private:
    Map &map;          // a map to dig
    int roomNum;       // room number
    int lastx, lasty;  // center of the last room

   public:
    BspListener(Map &map) : map(map), roomNum(0) {
        SPDLOG_TRACE("BspListener created");
    }

    bool visitNode(TCODBsp *node, void *user_data) {
        (void)user_data;  // don't need user_data
        if(node->isLeaf()) {
            long x, y, w, h;
            // dig a room
            TCODRandom *rng = TCODRandom::getInstance();
            w = rng->getInt(ROOM_MIN_SIZE, node->w - 2);
            h = rng->getInt(ROOM_MIN_SIZE, node->h - 2);
            x = rng->getInt(node->x + 1, node->x + node->w - w - 1);
            y = rng->getInt(node->y + 1, node->y + node->h - h - 1);
            SPDLOG_DEBUG("Creating room {} at ({}, {}) to ({}, {})", roomNum, x, y, x + w - 1, y + h - 1);
            map.createRoom(roomNum == 0, x, y, x + w - 1, y + h - 1);
            if(roomNum != 0) {
                SPDLOG_DEBUG("Digging corridor from ({}, {}) to ({}, {})", lastx, lasty, x + w / 2, lasty);
                // dig a corridor from last room
                map.dig(lastx, lasty, x + w / 2, lasty);
                map.dig(x + w / 2, lasty, x + w / 2, y + h / 2);
            }
            lastx = x + w / 2;
            lasty = y + h / 2;
            roomNum++;
        }
        return true;
    }
};

struct RandomMapBuilder {
    void operator()(Map &map) {
        SPDLOG_INFO("Building random map at {}x{} - max room size: {} max ratio: {}", map.width, map.height, ROOM_MAX_SIZE, 1.5f);
        TCODBsp bsp(0, 0, map.width, map.height);  // bsp is binary space partition tree
        // this creates the room partitions in our map
        int nb = 8;  // max level of recursion -- can make 2^nb rooms.
        bsp.splitRecursive(NULL, nb, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
        BspListener listener(map);
        bsp.traverseInvertedLevelOrder(&listener, NULL);
    }
};

struct RoomRect {
    long x1, y1, w, h, x2, y2;
    long center_x, center_y;
    RoomRect(long x1, long y1, long w, long h) : x1(x1), y1(y1), w(w), h(h),
        x2(x1 + w - 1), y2(y1 + h - 1),
        center_x(x1 + w / 2), center_y(y1 + h / 2) {
            SPDLOG_TRACE("New Room Rect: ({}, {}) -> {}x{}", x1, y1, w, h);
        }
    
    static RoomRect from_top_left(long x1, long y1, long w, long h) {
        SPDLOG_TRACE("Creating room from top left: {}, {}, {}, {}", x1, y1, w, h);
        return RoomRect(x1, y1, w, h);
    }
    
    static RoomRect from_center(long center_x, long center_y, long w, long h) {
        SPDLOG_TRACE("Creating room from center: {}, {}, {}, {}", center_x, center_y, w, h);
        return RoomRect(center_x - w / 2, center_y - h / 2, w, h);
    }
    
    static RoomRect from_corners(long x1, long y1, long x2, long y2) {
        SPDLOG_TRACE("Creating room from corners: {}, {}, {}, {}", x1, y1, x2, y2);
        return RoomRect(x1, y2, x2 - x1 + 1, y2 - y1 + 1);
    }
};

class FileMapBuilder {
    std::vector<RoomRect> rooms;
    std::vector<std::tuple<long, long>> corridors;
    std::function<RoomRect(long, long, long, long)> rect_parser;
public:
    FileMapBuilder(const std::string &filename) {
        load_file(filename);
    }

    void load_file(const std::string &filename) {
        SPDLOG_INFO("Loading map from file: {}", filename);
        // load the map from a file
        std::ifstream file(filename);
        if (!file) {
            SPDLOG_ERROR("Unable to open file: {}", filename);
            return;
        }
        SPDLOG_DEBUG("Successfully opened file: {}", filename);

        // get the read mode
        std::string read_mode; // CENTER or TOP_LEFT or CORNERS
        file >> read_mode;
        rect_parser = read_mode == "CENTER" ? RoomRect::from_center : (read_mode == "CORNERS" ? RoomRect::from_corners : RoomRect::from_top_left);
        SPDLOG_DEBUG("Rect read mode for the file: {}", read_mode);

        // get the room and corridor count
        long room_count, corridor_count;
        file >> room_count >> corridor_count;
        SPDLOG_DEBUG("Room count: {}, Corridor count: {}", room_count, corridor_count);

        // read the rooms
        SPDLOG_DEBUG("Reading rooms");
        for (long i = 0; i <  room_count; ++i) {
            long x, y, w, h;
            file >> x >> y >> w >> h;
            rooms.push_back(rect_parser(x, y, w, h));
        }

        // read the corridors
        SPDLOG_DEBUG("Reading corridors");
        for (long i = 0; i <  corridor_count; ++i) {
            long room_idx1, room_idx2;
            file >> room_idx1 >> room_idx2;
            corridors.push_back(std::make_pair(room_idx1, room_idx2));
        }
    }
    void operator()(Map &map) {
        SPDLOG_INFO("Building map from file");

        SPDLOG_DEBUG("Creating rooms");
        bool first = true;
        for (auto &room : rooms) {
            map.createRoom(first, room.x1, room.y1, room.x2, room.y2);
            first = false;
        }

        SPDLOG_DEBUG("Creating corridors");
        for (auto &corridor : corridors) {
            long room_idx1, room_idx2;
            std::tie(room_idx1, room_idx2) = corridor;
            SPDLOG_TRACE("Creating corridor between rooms {} and {}", room_idx1, room_idx2);
            RoomRect &r1 = rooms[room_idx1];
            RoomRect &r2 = rooms[room_idx2];

            bool vertical_gap = r1.y2 < r2.y1 || r2.y2 < r1.y1;
            bool horizontal_gap = r1.x2 < r2.x1 || r2.x2 < r1.x1;
            SPDLOG_TRACE("Vertical gap: {}, Horizontal gap: {}", vertical_gap, horizontal_gap);

            if (vertical_gap && horizontal_gap) {
                // no overlap - L-shaped corridor
                SPDLOG_TRACE("Digging corridor between room {} and {} (L-shaped) - {}, {} -> {}, {}", room_idx1, room_idx2, r1.x1, r1.y1, r2.x1, r2.y1);
                map.dig(r1.center_x, r1.center_y, r2.center_x, r1.center_y);
                map.dig(r2.center_x, r1.center_y, r2.center_x, r2.center_y);
            } else if (vertical_gap) {
                // no overlap - vertical gap
                long min_x = std::max(r1.x1, r2.x1), max_x = std::min(r1.x2, r2.x2);
                long center_x = (min_x + max_x) / 2;
                SPDLOG_TRACE("Digging corridor between room {} and {} (vertical) - {}, {} -> {}, {}", room_idx1, room_idx2, center_x, r1.center_y, center_x, r2.center_y);
                map.dig(center_x, r1.center_y, center_x, r2.center_y);
            } else if (horizontal_gap) {
                // no overlap - horizontal gap
                long min_y = std::max(r1.y1, r2.y1), max_y = std::min(r1.y2, r2.y2);
                long center_y = (min_y + max_y) / 2;
                SPDLOG_TRACE("Digging corridor between room {} and {} (horizontal) - {}, {} -> {}, {}", room_idx1, room_idx2, r1.center_x, center_y, r2.center_x, center_y);
                map.dig(r1.center_x, center_y, r2.center_x, center_y);
            } else {
                // overlap
                SPDLOG_ERROR("Overlapping rooms not supported between room {} and {}", room_idx1, room_idx2);
            }
        }
        SPDLOG_DEBUG("Map built from file");
    }
};

Map::Map(int width, int height, std::vector<Ant *> &ants,
         std::vector<Building *> &buildings, ProjectArguments &config) :
      width(width),
      height(height),
      ants(ants),
      buildings(buildings),
      tiles(new Tile[width * height]),
      map(new TCODMap(width, height)) {
        SPDLOG_INFO("Creating map of size {}x{}", width, height);
        if (config.default_map_file_path.empty()) build_map = RandomMapBuilder();
        else build_map = FileMapBuilder(config.default_map_file_path);
        SPDLOG_DEBUG("Map created");
      }

Map::~Map() {
    SPDLOG_INFO("Destroying map");
    delete[] tiles;
    delete map;
    SPDLOG_DEBUG("Map destroyed");
}

void Map::build() { build_map(*this); }

Tile &Map::getTile(long x, long y) const {
    SPDLOG_TRACE("Getting tile at ({}, {})", x, y);
    return tiles[x + y * width];
}

void Map::setWall(long x, long y) {
    SPDLOG_TRACE("Setting wall at ({}, {})", x, y);
    map->setProperties(x, y, false, false);
}

bool Map::isWall(long x, long y) const {
    SPDLOG_TRACE("Checking if wall at ({}, {}): {}", x, y, !map->isWalkable(x, y));
    return !map->isWalkable(x, y);
}

bool Map::canWalk(long x, long y) const {
    SPDLOG_TRACE("Checking if can walk at ({}, {})", x, y);
    if(x < 0 || x >= width || y < 0 || y >= height){
        SPDLOG_DEBUG("Cannot walk - out of bounds");
        return false;
    }
    if(isWall(x, y)) {
        SPDLOG_DEBUG("Cannot walk - wall");
        return false;
    }
    SPDLOG_TRACE("Checking if actor will collide with another ant at ({}, {})");
    for(auto ant : ants) {
        if(ant->x == x && ant->y == y) {
            SPDLOG_DEBUG("Cannot walk - ant collision");
            // there is an actor there. cannot walk
            return false;
        }
    }
    SPDLOG_DEBUG("Ant can walk");
    return true;
}

bool Map::isInFov(long x, long y) const {
    Tile &tile = getTile(x, y);
    if(tile.inFov) {
        SPDLOG_TRACE("Tile at ({}, {}) is in FOV - setting to explored", x, y);
        tile.explored = true;
        return true;
    }
    SPDLOG_TRACE("Tile at ({}, {}) is not in FOV", x, y);
    return false;
}

bool Map::isExplored(long x, long y) const {
    SPDLOG_TRACE("Checking if tile at ({}, {}) is explored", x, y); 
    return getTile(x, y).explored;
}

void Map::resetFov() {
    SPDLOG_DEBUG("Resetting FOV on map");
    for(long x = 0; x < width; x++) {
        for(long y = 0; y < height; y++) {
            getTile(x, y).inFov = false;
        }
    }
    SPDLOG_DEBUG("Resetting FOV on ants");
    for(auto ant : ants) {
        ant->resetFov();
    }
    SPDLOG_TRACE("FOV reset");
}

void Map::updateTileFov() {
    SPDLOG_DEBUG("Updating FOV on map tiles");
    for(long x = 0; x < width; x++) {
        for(long y = 0; y < height; y++) {
            if(map->isInFov(x, y)) {
                getTile(x, y).inFov = true;
            }
        }
    }
    SPDLOG_TRACE("FOV updated for the map tiles");
}

void Map::updateFov() {
    SPDLOG_DEBUG("Updating FOV on map");
    resetFov();

    SPDLOG_DEBUG("Updating FOV on ants");
    for(auto ant : ants) {
        map->computeFov(ant->x, ant->y, ant->fovRadius);
        updateTileFov();
    }
    SPDLOG_TRACE("FOV on map updated");
}

/*
 * dig makes a rectangle of map specified by two x,y points as walkable
 */
void Map::dig(long x1, long y1, long x2, long y2) {
    SPDLOG_DEBUG("Digging from ({}, {}) to ({}, {})", x1, y1, x2, y2);
    if(x2 < x1) {
        int tmp = x2;
        x2 = x1;
        x1 = tmp;
    }
    if(y2 < y1) {
        int tmp = y2;
        y2 = y1;
        y1 = tmp;
    }
    SPDLOG_TRACE("Setting properties for tiles");
    for(int tilex = x1; tilex <= x2; tilex++) {
        for(int tiley = y1; tiley <= y2; tiley++) {
            map->setProperties(tilex, tiley, true, true);
        }
    }
    SPDLOG_TRACE("Digging complete");
}

// TODO: make a table which keeps track of rooms
// As rooms are constructed, add room dimensions to table
// Then, once all rooms are constructed, we can add in objects, monsters,
// buildings, and ants This should stop ants and buildnigs references from being
// passed intp Map class
void Map::createRoom(bool first, long x1, long y1, long x2, long y2) {
    SPDLOG_DEBUG("Creating room from ({}, {}) to ({}, {})", x1, y1, x2, y2);
    dig(x1, y1, x2, y2);

    if(first) {
        SPDLOG_DEBUG("Creating nursery in first room");
        int center_x = (x1 + x2) / 2, center_y = (y1 + y2) / 2;
        // put the player in the first room
        ants[0]->x = center_x;
        ants[0]->y = center_y;
        SPDLOG_DEBUG("Placing player at ({}, {})", center_x, center_y);
        buildings.push_back(new Nursery(center_x, center_y, 0, *this));
    }
    SPDLOG_TRACE("Room created");
}
