#pragma once

#include <unordered_set>

#include "hardware/brain.hpp"
#include "hardware/program_executor.hpp"
#include "hardware/command_config.hpp"
#include "entity/entity_data.hpp"

struct Player: public MapEntity {
    EntityData data;

    Player(EntityData const& data);
    Player(Unpacker& p);
    EntityData& get_data();
    ~Player()=default;
    void move_callback(long x, long y, long new_x, long new_y);
    void click_callback(long x, long y);
    MapEntityType get_type() const;

    friend Packer& operator<<(Packer& p, Player const& obj);
};

struct Worker: public MapEntity {
    EntityData data;
    ProgramExecutor program_executor;
    DualRegisters cpu;
    ushort move_speed = 12; // 60 FPS / 5 moves per sec = 12

    std::unordered_set<CommandEnum> command_set = {
        CommandEnum::ADD, CommandEnum::DEC, CommandEnum::INC,
        CommandEnum::JMP, CommandEnum::JNZ, CommandEnum::LOAD,
        CommandEnum::MOVE, CommandEnum::NOP, CommandEnum::SUB,
        CommandEnum::COPY
    };

    Worker(EntityData const& data);
    Worker(Unpacker& p);
    ~Worker()=default;

    EntityData& get_data();
    void move_callback(long x, long y, long new_x, long new_y);
    void click_callback(long x, long y);
    MapEntityType get_type() const;

    friend Packer& operator<<(Packer& p, Worker const& obj);
};
