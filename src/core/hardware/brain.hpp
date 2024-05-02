#pragma once
#include "app/globals.hpp"
#include <stddef.h>

struct DualRegisters {
    cpu_word_size registers[2];
    bool zero_flag;
    cpu_word_size& operator[](size_t idx) { return registers[idx]; }
    cpu_word_size const& operator[](size_t idx) const { return registers[idx]; }
};
