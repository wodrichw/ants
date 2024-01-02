#ifndef  __MAP_HPP
#define  __MAP_HPP

 #include <libtcod/console_types.hpp>
__MAP_HPP
#include <libtcod.hpp>
#include <vector>
#include "ant.hpp"


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
    tcod::Console root_console;

    Map(int width, int height, std::vector<ant::Ant*>& ants, std::vector<Building*>&buildings);
    ~Map();
    Tile& getTile(int x, int y) const;
    void clearCh(int x, int y);
    bool isWall(int x, int y) const;
    bool canWalk(int x, int y) const;
    bool isInFov(int x, int y) const; // Fov = field of view
    bool isExplored(int x, int y) const;
    void render();
    void renderAnt(ant::Ant& a);
    void renderBuilding(Building& b);
    void updateFov();
private :
    std::vector<ant::Ant*>& ants;
    std::vector<Building*>& buildings;
    Tile *tiles;
    TCODMap *map;
    friend class BspListener;

    void dig(int x1, int y1, int x2, int y2);
    void createRoom(bool first, int x1, int y1, int x2, int y2);
    void setWall(int x, int y);
    void resetFov();
    void updateTileFov();
};


#endif //__MAP_HPP
