#pragma once

#include <vector>

#include "hardware/program_executor.hpp"
#include "utils/status.hpp"

using uchar = unsigned char;

struct DualRegisters;

struct CompileArgs {
    std::vector<uchar> const& code;
    std::vector<uchar>::const_iterator code_it;
    DualRegisters& cpu;
    std::vector<Op>& ops;
    Status status;

    CompileArgs(std::vector<uchar> const& code, DualRegisters& cpu,
                std::vector<Op>& ops)
        : code(code), code_it(code.begin()), cpu(cpu), ops(ops) {}
};
