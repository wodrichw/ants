#pragma once

#include <vector>

#include <hardware/compiler.hpp>

class ClockController;

struct HardwareManager {
    std::vector<ClockController*> controllers;
    Compiler compiler;

    HardwareManager(CommandMap const& command_map): compiler(command_map) {};
};
