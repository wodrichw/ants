#include "entity/ant.hpp"

#include <libtcod/color.hpp>
#include <optional>

#include "ui/colors.hpp"
#include "entity/entity_data.hpp"
#include "proto/entity.pb.h"
#include "spdlog/spdlog.h"

Player::Player(EntityData const& data)
    : data(data) {
        SPDLOG_INFO("Player created at ({}, {})", data.x, data.y);
}

Player::Player(Unpacker& p) : data(p) {}

EntityData& Player::get_data() {
    return data;
}

MapEntityType Player::get_type() const { return PLAYER; }

Packer& operator<<(Packer& p, Player const& obj) {
    return p << obj.data;
}

Worker::Worker(EntityData const& data)
    : data(data), program_executor(), cpu() {
}

Worker::Worker(Unpacker& p): data(p), program_executor(p), cpu(p) {}

EntityData& Worker::get_data() {
    return data;
}

Packer& operator<<(Packer& p, Worker const& obj) {
    return p << obj.data << obj.program_executor << obj.cpu;
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
