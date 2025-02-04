#pragma once

#include <unordered_set>

#include "entity.pb.h"
#include "entity/entity_data.hpp"
#include "entity/inventory.hpp"
#include "hardware/brain.hpp"
#include "hardware/command_config.hpp"
#include "hardware/program_executor.hpp"
#include "utils/thread_pool.hpp"

struct Player : public MapEntity {
    EntityData data;
    Inventory inventory;

    Player(EntityData const& data, ItemInfoMap const&);
    Player(const ant_proto::Player& msg, ItemInfoMap const&);
    EntityData& get_data();
    ~Player() = default;
    void request_move();
    void move_callback(EntityMoveUpdate const&);
    void click_callback(long x, long y);
    void handle_empty_space(uchar bits);
    void handle_full_space(uchar bits);
    MapEntityType get_type() const;

    ant_proto::Player get_proto() const;
};

struct Worker : public MapEntity {
    EntityData data;
    ulong max_instruction_per_tick = 500;
    DualRegisters cpu = {};
    ProgramExecutor program_executor;
    Inventory inventory;

    std::unordered_set<CommandEnum> command_set = {
        CommandEnum::ADD,  CommandEnum::DEC, CommandEnum::INC,
        CommandEnum::JMP,  CommandEnum::JNZ, CommandEnum::LOAD,
        CommandEnum::MOVE, CommandEnum::NOP, CommandEnum::SUB,
        CommandEnum::COPY};

    Worker(EntityData const& data, ulong const& instr_clock, ItemInfoMap const&,
           ThreadPool<AsyncProgramJob>&);
    Worker(const ant_proto::Worker& msg, ulong const& instr_clock,
           ItemInfoMap const&, ThreadPool<AsyncProgramJob>&);
    ~Worker() = default;

    EntityData& get_data();
    void request_move();
    void move_callback(EntityMoveUpdate const&);
    void click_callback(long x, long y);
    void handle_empty_space(uchar bits);
    void handle_full_space(uchar bits);
    MapEntityType get_type() const;

    ant_proto::Worker get_proto();

   private:
    void debug_empty_space_flags();
};
