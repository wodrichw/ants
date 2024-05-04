#pragma once

#include <stddef.h>

#include "app/globals.hpp"
#include "utils/serializer.hpp"

struct DualRegisters {
    cpu_word_size registers[2];
    bool zero_flag;
    cpu_word_size& operator[](size_t idx) { return registers[idx]; }
    cpu_word_size const& operator[](size_t idx) const { return registers[idx]; }

    friend Packer& operator<<(Packer& packer, DualRegisters const& obj) {
        return packer << obj.registers[0] << obj.registers[1] << obj.zero_flag;
    }
    friend Unpacker& operator<<(Unpacker& unpacker, DualRegisters& obj) {
        return unpacker >> obj.registers[0] >> obj.registers[1] >> obj.zero_flag;
    }
};
