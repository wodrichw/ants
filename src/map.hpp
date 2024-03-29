#pragma once

#include <libtcod/console_types.hpp>
#include <libtcod.hpp>
#include <vector>

class Ant;
class Building;

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;

struct Tile {
    bool explored; // has the player already seen this tile ?
    bool inFov;
    std::optional<int> bldgId; // set if to the ID of the building occupying the tile, unset if no building
    Tile() : explored(false), inFov(false), bldgId() {}
};

class Map {
public :
    int width,height;

    Map(int width, int height, std::vector<Ant*>& ants, std::vector<Building*>&buildings);
    ~Map();
    void build();
    Tile& getTile(long x, long y) const;
    bool isWall(long x, long y) const;
    bool canWalk(long x, long y) const;
    bool isInFov(long x, long y) const; // Fov = field of view
    bool isExplored(long x, long y) const;

    void updateFov();
private :
    std::vector<Ant*>& ants;
    std::vector<Building*>& buildings;
    Tile *tiles;
    TCODMap *map;
    friend class BspListener;

    void dig(long x1, long y1, long x2, long y2);
    void createRoom(bool first, long x1, long y1, long x2, long y2);
    void setWall(long x, long y);
    void resetFov();
    void updateTileFov();
};
