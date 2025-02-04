#pragma once

#include "map/map.hpp"

using schar = signed char;

// Helper function to get scent value from chunk
ulong get_scent(ulong const& scents, ulong bit_idx);

// Helper function to update scent and sum
void update_scent_and_sum(long& total, ulong& scents, long priority);

// =============================================================================
// Scent Writing
// =============================================================================

template <typename Modifier>
class ScentWriter {
    ulong& delta_scents;
    uchar bit_idx = 0;  // multiple of 8

   public:
    ScentWriter(ulong& delta_scents, uchar scent_idx)
        : delta_scents(delta_scents), bit_idx(scent_idx * 8) {}
    void operator()() {
        // get scent
        ulong delta_scent = get_scent(delta_scents, bit_idx);

        Modifier()(delta_scent);

        // clear scent bits and readd it
        delta_scents &= ~(0xFF << bit_idx);
        delta_scents |= (delta_scent << bit_idx);
    }
};

struct ImmutableScentBehavior {
    void operator()() {}
};

struct DecrementScent {
    void operator()(ulong&);
};
struct IncrementScent {
    void operator()(ulong&);
};

using DecrementScentBehavior = ScentWriter<DecrementScent>;
using IncrementScentBehavior = ScentWriter<IncrementScent>;

// =============================================================================
// Scent Reading
// =============================================================================

class ScentReader {
    bool &scent_dir1, &scent_dir2;
    uchar const& is_space_empty_flag;
    ulong const& base_priorities;

   public:
    ScentReader(bool& scent_dir1, bool& dir_flag2,
                uchar const& is_space_empty_flag, ulong const& priorities);
    void operator()(ulong abs_scents[4]);
};

struct ScentBehaviors {
    ScentReader read_scent_behavior;
    std::function<void()> write_scent_behavior = ImmutableScentBehavior();
    bool scent_dir1 = false, scent_dir2 = false;
    ulong priorities = 0;

    ScentBehaviors(uchar const& is_space_empty_flag);
};
