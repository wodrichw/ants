#pragma once

#include "hardware/brain.hpp"
#include "hardware/operations.hpp"
#include "entity/map_entity.hpp"

struct Player: public MapEntity {
    EntityData data;

    Player(EntityData const& data);
    EntityData& get_data();
    ~Player()=default;
    void move_callback(long x, long y, long new_x, long new_y);
    void click_callback(long x, long y);
    
};

struct Worker: public MapEntity {
    EntityData data;
    Operations operations;
    DualRegisters cpu;

    Worker(EntityData const& data);
    ~Worker()=default;
    EntityData& get_data();
    void move_callback(long x, long y, long new_x, long new_y);
    void click_callback(long x, long y);
};
