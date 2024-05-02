#pragma once

#include "hardware/brain.hpp"
#include "ui/buttonController.hpp"
#include "hardware/operations.hpp"
#include "ui/subscriber.hpp"
#include "entity/map_entity.hpp"

struct Player: public MapEntity {
    MapData data;

    Player(MapData const& data);
    MapData& get_data();
    ~Player()=default;
    void move_callback(long x, long y, long new_x, long new_y);
    void click_callback(long x, long y);
    
};

struct Worker: public MapEntity {
    MapData data;
    Operations operations;
    DualRegisters cpu;

    Worker(MapData const& data);
    ~Worker()=default;
    MapData& get_data();
    void move_callback(long x, long y, long new_x, long new_y);
    void click_callback(long x, long y);
};