#pragma once

#include <functional>
#include "globals.hpp"

struct AntInteractor {
    std::function<void(long dx, long dy)> try_move;
    std::function<cpu_word_size const&(long idx)> read_register;
    std::function<void(long idx, cpu_word_size value)> write_register;
    std::function<bool const&()> read_zero_flag;
    std::function<void(bool flag)> write_zero_flag;
};