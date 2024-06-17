#include "entity/ant.hpp"

#include <libtcod/color.hpp>
#include <optional>

#include "ui/colors.hpp"
#include "entity/entity_data.hpp"
#include "proto/entity.pb.h"
#include "spdlog/spdlog.h"

Player::Player(EntityData const& data, ItemInfoMap const& info_map)
    : data(data), inventory(1,1, 1000, info_map) {
        SPDLOG_INFO("Player created at ({}, {})", data.x, data.y);
}

Player::Player(Unpacker& p, ItemInfoMap const& info_map) : data(p), inventory(p, info_map) {
    SPDLOG_TRACE("Completed unpacking player");
}

EntityData& Player::get_data() {
    return data;
}

MapEntityType Player::get_type() const { return PLAYER; }

void Player::request_move() {}

Packer& operator<<(Packer& p, Player const& obj) {
    SPDLOG_DEBUG("Packing player");
    return p << obj.data << obj.inventory;
}

Worker::Worker(EntityData const& data, ulong const& instr_clock, ItemInfoMap const& info_map)
    : data(data), program_executor(instr_clock), cpu(), inventory(1, 1, 1000, info_map) {
}

Worker::Worker(Unpacker& p, ulong const& instr_clock, ItemInfoMap const& info_map): data(p), program_executor(p, instr_clock), cpu(p), inventory(p, info_map) {
    SPDLOG_TRACE("Completed unpacking worker");
}

EntityData& Worker::get_data() {
    return data;
}

void Worker::request_move() { program_executor.handleClockPulse(); }

Packer& operator<<(Packer& p, Worker const& obj) {
    SPDLOG_TRACE("Packing worker");
    return p << obj.data << obj.program_executor << obj.cpu << obj.inventory;
}

void toggle_color(tcod::ColorRGB& col) {
    if(col == color::light_green){
        col = color::dark_yellow;
        SPDLOG_DEBUG("Worker toggled color to dark yellow");
    }else{
        col = color::light_green;
        SPDLOG_DEBUG("Worker toggled color to light green");
    }
}

void Player::click_callback(long, long) {
    SPDLOG_INFO("Player clicked - toggling color");
    toggle_color(data.col);
}

void Player::move_callback(long, long, long, long) {}

void Worker::click_callback(long, long) {
    SPDLOG_INFO("Worker clicked - toggling color");
    toggle_color(data.col);
}

void Worker::move_callback(long, long, long, long) {}

MapEntityType Worker::get_type() const { return WORKER; }
