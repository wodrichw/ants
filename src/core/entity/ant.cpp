#include "entity/ant.hpp"

#include <libtcod/color.hpp>
#include <optional>

#include "ui/colors.hpp"
#include "spdlog/spdlog.h"

Player::Player(MapData const& data)
    : data(data) {
        SPDLOG_INFO("Player created at ({}, {})", data.x, data.y);
}

MapData& Player::get_data() {
    return data;
}

Worker::Worker(MapData const& data)
    : data(data), operations(), cpu() {
}

MapData& Worker::get_data() {
    return data;
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
