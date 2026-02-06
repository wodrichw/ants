#include "entity/scents.hpp"

#include <math.h>
#include <stdlib.h>

#include "utils/types.hpp"
#include "spdlog/spdlog.h"

ulong get_scent(ulong const& scents, ulong bit_idx) {
    return (scents >> bit_idx) & 0xFF;
}

// Function to calculate softmax probabilities and select a direction
long select_direction(ulong scents[4], ulong weights, uchar is_empty) {
    // Extract weights into an array of signed integers
    std::cout << "\nSTART" << std::endl;
    long weight_array[8];
    std::cout << "Weights: ";
    for(int i = 0; i < 8; i++) {
        weight_array[i] = (long)(signed char)((weights >> (i * 8)) & 0xFF);
        std::cout << weight_array[i] << " ";
    }
    std::cout << std::endl;

    // Compute scores for each direction
    double scores[4] = {0};
    double NEG_INFINITIY = -1e18;
    double max_score = NEG_INFINITIY;

    std::cout << "Scores:" << std::endl;
    for(int dir = 0; dir < 4; dir++) {
        if((is_empty & (1 << dir)) == 0) {
            // If the direction is not empty, set score to -INFINITY
            scores[dir] = NEG_INFINITIY;
            continue;
        }

        for(int i = 0; i < 8; i++) {
            int scent = (scents[dir] >> (i * 8)) & 0xFF;
            scores[dir] += scent * weight_array[i];
            std::cout << scent << " ";
        }
        std::cout << "--> " << scores[dir] << std::endl;

        max_score = std::max(max_score, scores[dir]);
    }

    // Compute the softmax probabilities
    double exp_scores[4] = {0};
    double sum_exp = 0;

    for(ulong dir = 0; dir < 4; dir++) {
        exp_scores[dir] = exp(scores[dir] - max_score);  // Stability adjustment
        sum_exp += exp_scores[dir];
    }

    // Normalize probabilities
    double probabilities[4] = {0};
    for(ulong dir = 0; dir < 4; dir++) {
        probabilities[dir] = exp_scores[dir] / sum_exp;
    }

    // Select a direction based on the probabilities
    double random_value = ((double)rand() / RAND_MAX);
    double cumulative_probability = 0;

    for(ulong dir = 0; dir < 4; dir++) {
        cumulative_probability += probabilities[dir];
        if(random_value <= cumulative_probability) {
            SPDLOG_INFO("Selected: {}", dir);
            return static_cast<long>(dir);
        }
    }

    return -1;  // Should not reach here if probabilities are calculated
                // correctly
}

void DecrementScent::operator()(ulong& delta_scent) {
    delta_scent = (delta_scent + 0xFF) &
                  0xFF;  // equivalent to --scent since this will truncated
    // return true;
}

void IncrementScent::operator()(ulong& delta_scent) {
    delta_scent = (delta_scent + 1) & 0xFF;
}

ScentReader::ScentReader(bool& scent_dir1, bool& dir_flag2,
                         uchar const& is_space_empty_flag,
                         ulong const& priorities)
    : scent_dir1(scent_dir1),
      scent_dir2(dir_flag2),
      is_space_empty_flag(is_space_empty_flag),
      base_priorities(priorities) {}

void ScentReader::operator()(ulong abs_scents[4]) {
    long direction =
        select_direction(abs_scents, base_priorities, is_space_empty_flag);
    if(direction < 0) return;

    scent_dir1 = (direction & 2) != 0;
    scent_dir2 = (direction & 1) != 0;
}

ScentBehaviors::ScentBehaviors(uchar const& is_space_empty_flag)
    : read_scent_behavior(scent_dir1, scent_dir2, is_space_empty_flag,
                          priorities) {}
