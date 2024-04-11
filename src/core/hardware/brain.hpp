#include "app/globals.hpp"

struct DualRegisters {
    cpu_word_size registers[2];
    bool zero_flag;
};
