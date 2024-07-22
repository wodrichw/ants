#pragma once

#include <unordered_set>

#include "hardware/brain.hpp"
#include "hardware/program_executor.hpp"
#include "hardware/command_config.hpp"
#include "entity/entity_data.hpp"
#include "entity/inventory.hpp"
#include "utils/thread_pool.hpp"

struct Player: public MapEntity {
    EntityData data;
    Inventory inventory;

    Player(EntityData const& data, ItemInfoMap const&);
    Player(Unpacker& p, ItemInfoMap const&);
    EntityData& get_data();
    ~Player()=default;
    void request_move();
    void move_callback(long x, long y, long new_x, long new_y);
    void click_callback(long x, long y);
    MapEntityType get_type() const;

    friend Packer& operator<<(Packer& p, Player const& obj);
};

struct Worker: public MapEntity {
    EntityData data;
    ulong  max_instruction_per_tick = 500;
    DualRegisters cpu = {};
    ProgramExecutor program_executor;
    Inventory inventory;

    std::unordered_set<CommandEnum> command_set = {
        CommandEnum::ADD, CommandEnum::DEC, CommandEnum::INC,
        CommandEnum::JMP, CommandEnum::JNZ, CommandEnum::LOAD,
        CommandEnum::MOVE, CommandEnum::NOP, CommandEnum::SUB,
        CommandEnum::COPY
    };

    Worker(EntityData const& data, ulong const& instr_clock, ItemInfoMap const&, ThreadPool<AsyncProgramJob>&);
    Worker(Unpacker& p, ulong const& instr_clock, ItemInfoMap const&, ThreadPool<AsyncProgramJob>&);
    ~Worker()=default;

    EntityData& get_data();
    void request_move();
    void move_callback(long x, long y, long new_x, long new_y);
    void click_callback(long x, long y);
    MapEntityType get_type() const;

    friend Packer& operator<<(Packer& p, Worker const& obj);
};

