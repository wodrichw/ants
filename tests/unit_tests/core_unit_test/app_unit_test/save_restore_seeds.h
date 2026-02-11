#pragma once

#include <cstdint>
#include <limits>
#include <random>
#include <unordered_set>
#include <vector>

struct SaveRestoreSeed {
    uint32_t seed_x;
    uint32_t seed_y;
};

inline constexpr std::size_t kSaveRestoreSeedCount = 100;
inline constexpr uint32_t kSaveRestoreBaseSeed = 1337u;

inline std::vector<SaveRestoreSeed> fixed_save_restore_seeds() {
    return {
        {0u, 0u},
        {1u, 1u},
        {2u, 3u},
        {13u, 37u},
        {42u, 99u},
        {123u, 456u},
        {1024u, 2048u},
        {65535u, 65535u},
        {std::numeric_limits<uint32_t>::max(), 0u},
        {std::numeric_limits<uint32_t>::max(),
         std::numeric_limits<uint32_t>::max()},
    };
}

inline std::vector<SaveRestoreSeed> all_save_restore_seeds() {
    std::vector<SaveRestoreSeed> seeds = fixed_save_restore_seeds();
    std::unordered_set<uint64_t> used;
    used.reserve(kSaveRestoreSeedCount * 2);

    for(const auto& seed : seeds) {
        uint64_t key = (static_cast<uint64_t>(seed.seed_x) << 32) | seed.seed_y;
        used.insert(key);
    }

    std::mt19937 rng(kSaveRestoreBaseSeed);
    std::uniform_int_distribution<uint32_t> dist(
        0u, std::numeric_limits<uint32_t>::max());

    while(seeds.size() < kSaveRestoreSeedCount) {
        uint32_t seed_x = dist(rng);
        uint32_t seed_y = dist(rng);
        uint64_t key = (static_cast<uint64_t>(seed_x) << 32) | seed_y;
        if(used.insert(key).second) {
            seeds.push_back(SaveRestoreSeed{seed_x, seed_y});
        }
    }

    return seeds;
}
