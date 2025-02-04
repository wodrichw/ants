#include "entity/ant.hpp"

#include <libtcod/color.hpp>

#include "entity.pb.h"
#include "entity/entity_data.hpp"
#include "hardware/program_executor.hpp"
#include "spdlog/spdlog.h"
#include "ui/colors.hpp"
#include "utils/thread_pool.hpp"

Player::Player(EntityData const& data, ItemInfoMap const& info_map)
    : data(data), inventory(1, 1, 1000, info_map) {
    SPDLOG_INFO("Player created at ({}, {})", data.x, data.y);
}

Player::Player(const ant_proto::Player& msg, ItemInfoMap const& info_map)
    : data(msg.data()), inventory(msg.inventory(), info_map) {
    SPDLOG_TRACE("Completed unpacking player");
}

EntityData& Player::get_data() { return data; }

MapEntityType Player::get_type() const { return PLAYER; }

void Player::request_move() {}
void Player::handle_empty_space(uchar) {}
void Player::handle_full_space(uchar) {}

ant_proto::Player Player::get_proto() const {
    ant_proto::Player msg;
    *msg.mutable_data() = data.get_proto();
    *msg.mutable_inventory() = inventory.get_proto();
    return msg;
}

Worker::Worker(EntityData const& data, ulong const& instr_clock,
               ItemInfoMap const& info_map,
               ThreadPool<AsyncProgramJob>& job_pool)
    : data(data),
      cpu(),
      program_executor(instr_clock, max_instruction_per_tick,
                       cpu.instr_ptr_register, job_pool),
      inventory(1, 1, 1000, info_map) {}

Worker::Worker(const ant_proto::Worker& msg, ulong const& instr_clock,
               ItemInfoMap const& info_map,
               ThreadPool<AsyncProgramJob>& job_pool)
    : data(msg.data()),
      cpu(msg.dual_registers()),
      program_executor(msg.program_executor(), instr_clock,
                       max_instruction_per_tick, cpu.instr_ptr_register,
                       job_pool),
      inventory(msg.inventory(), info_map) {
    SPDLOG_TRACE("Completed unpacking worker");
}

EntityData& Worker::get_data() { return data; }

void Worker::request_move() { program_executor.execute_sync(); }

void Worker::handle_empty_space(uchar bits) {
    cpu.is_space_empty_flags |= bits;
}

void Worker::handle_full_space(uchar bits) {
    cpu.is_space_empty_flags &= ~bits;
}

ant_proto::Worker Worker::get_proto() {
    ant_proto::Worker msg;
    *msg.mutable_data() = data.get_proto();
    *msg.mutable_inventory() = inventory.get_proto();
    *msg.mutable_program_executor() = program_executor.get_proto();
    *msg.mutable_dual_registers() = cpu.get_proto();

    return msg;
}

void toggle_color(tcod::ColorRGB& col) {
    if(col == color::light_green) {
        col = color::dark_yellow;
        SPDLOG_DEBUG("Worker toggled color to dark yellow");
    } else {
        col = color::light_green;
        SPDLOG_DEBUG("Worker toggled color to light green");
    }
}

void Player::click_callback(long, long) {
    SPDLOG_INFO("Player clicked - toggling color");
    toggle_color(data.col);
}

void Player::move_callback(EntityMoveUpdate const&) {}

void Worker::click_callback(long, long) {
    SPDLOG_INFO("Worker clicked - toggling color");
    toggle_color(data.col);
}

void Worker::move_callback(EntityMoveUpdate const& update) {
    ScentBehaviors& scent_behaviors = cpu.scent_behaviors;
    scent_behaviors.read_scent_behavior((ulong*)update.abs_scents);
    scent_behaviors.write_scent_behavior();

    // SPDLOG_INFO("Move callback - worker - {}", cpu.is_space_empty_flags);
}

void Worker::debug_empty_space_flags() {
    SPDLOG_TRACE("Update worker full space flags: D:{} L:{} U:{} R:{}",
                 (cpu.is_space_empty_flags >> 3) & 1,
                 (cpu.is_space_empty_flags >> 2) & 1,
                 (cpu.is_space_empty_flags >> 1) & 1,
                 (cpu.is_space_empty_flags >> 0) & 1);
    // Debug empty space flags
    // DLUR
    std::string facings = "+6894-7^23|>1V<o";
    data.ch = facings[cpu.is_space_empty_flags];
}

MapEntityType Worker::get_type() const { return WORKER; }
