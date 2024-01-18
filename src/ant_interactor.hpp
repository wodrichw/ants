#pragma once

#include <functional>
#include "globals.hpp"

struct AntInteractor {
    std::function<void(long dx, long dy)> try_move;
    std::function<cpu_word_size const&(long idx)> read_register;
    std::function<void(long idx, cpu_word_size value)> write_register;
};