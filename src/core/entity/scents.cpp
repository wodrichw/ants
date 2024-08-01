#include "entity/scents.hpp"

using ulong = unsigned long;

ulong get_scent(ulong const& scents, ulong bit_idx) {
    return (scents >> bit_idx) & 0xFF;
}

void update_scent_and_sum(long& total, ulong& scents, long priority) {
    total += static_cast<long>(scents & 0xFF) * priority;
    scents >>= 8;
}

void DecrementScent::operator()(ulong& scent) {
    // if (scent == 0) return false; // not really needed since over/underflow is impossible
    scent = (scent + 0xFF) & 0xFF; // equivalent to --scent since this will truncated
    // return true;
}

void IncrementScent::operator()(ulong& scent) {
    // if (scent == 0) return false; // not really needed since over/underflow is impossible
    scent = (scent + 1) & 0xFF; // equivalent to ++scent since this will truncated
    // return true;
}

ScentReader::ScentReader(bool& scent_dir1, bool& dir_flag2, uchar const& is_space_empty_flag, ulong const& priorities)
      : scent_dir1(scent_dir1), scent_dir2(dir_flag2),
        is_space_empty_flag(is_space_empty_flag), base_priorities(priorities) {}

void ScentReader::operator()(ulong abs_scents[4]) {
    if (base_priorities == 0) return;

    // abs_scents - chunk directions: right, up, left, down
    ulong right_scents = abs_scents[0];
    ulong up_scents = abs_scents[1];
    ulong left_scents = abs_scents[2];
    ulong down_scents = abs_scents[3];

    // Horizontal - positive values push to the left
    // Vertical - positive values push forward
    long horizontal_weight = 0, vertical_weight = 0, max_value = 0;
    for (ulong priorities = base_priorities; priorities != 0; priorities >>= 8) {
        // Get and shift priorities
        long priority = static_cast<long>(static_cast<schar>(priorities & 0xFF));
        max_value += std::abs(priority) * 0xFF;

        // Get and shift left / right scents
        update_scent_and_sum(horizontal_weight, left_scents, priority);
        update_scent_and_sum(horizontal_weight, right_scents, -priority);

        // Get and shift forward /reverse scents
        update_scent_and_sum(vertical_weight, up_scents, priority);
        update_scent_and_sum(vertical_weight, down_scents, -priority);
    }

    if (horizontal_weight == 0 && vertical_weight == 0) {
        // randomize if no weights
        long rand_dir = rand();
        horizontal_weight = 2 * (rand_dir & 1) - 1;
        vertical_weight = 2 * ((rand_dir >> 1) & 1) - 1;
    }

    bool can_move_right = is_space_empty_flag & 1;
    bool can_move_up = (is_space_empty_flag >> 1 ) & 1;
    bool can_move_left = (is_space_empty_flag >> 2 ) & 1;
    bool can_move_down = (is_space_empty_flag >> 3 ) & 1;

    long left_scent = can_move_left ? (max_value + horizontal_weight) : 0;
    long right_scent = can_move_right ? (max_value - horizontal_weight) : 0;

    long up_scent = can_move_up ? (max_value + vertical_weight) : 0;
    long down_scent = can_move_down ? (max_value - vertical_weight) : 0;
    
    bool use_max = true;

    if(use_max) {
        long max_value = std::max({left_scent, right_scent, up_scent, down_scent});

        if (max_value == right_scent) scent_dir1 = false, scent_dir2 = false;
        else if (max_value == up_scent) scent_dir1 = false, scent_dir2 = true;
        else if (max_value == left_scent) scent_dir1 = true, scent_dir2 = false;
        else scent_dir1 = true, scent_dir1 = true;
        return;
    }

    // SPDLOG_INFO("Scent reader - left: {} right {} up: {} down: {}", left_scent, right_scent, up_scent, down_scent);

    long total_scent = left_scent + right_scent + up_scent + down_scent + 1;

    long rand_dir = rand() % total_scent;
    long accumulated_value = 0;

    accumulated_value += right_scent;
    if (rand_dir < accumulated_value) {
        // turn right;
        scent_dir1 = false, scent_dir2 = false;
        return;
    }
    accumulated_value += up_scent;
    if (rand_dir < accumulated_value) {
        // turn up;
        scent_dir1 = false, scent_dir2 = true;
        return;
    }
    accumulated_value += left_scent;
    if (rand_dir < accumulated_value) {
        // turn left;
        scent_dir1 = true, scent_dir2 = false;
        return;
    }
    // turn down;
    scent_dir1 = true, scent_dir2 = true;
}

ScentBehaviors::ScentBehaviors(uchar const& is_space_empty_flag):
    read_scent_behavior(scent_dir1, scent_dir2, is_space_empty_flag, priorities) {}
