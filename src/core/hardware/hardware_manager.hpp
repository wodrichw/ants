#pragma once

#include <vector>

#include <hardware/compiler.hpp>

class ClockController;

struct HardwareManager {
    private:
    using ControllerList = std::vector<ClockController*>;
    ControllerList controllers;
    Compiler compiler;

    public:
    HardwareManager(CommandMap const&);
    HardwareManager(Unpacker&, CommandMap const&);
    void push_back(ClockController*);
    void compile(MachineCode const& machine_code, AntInteractor& interactor, Status& status);

    ControllerList::iterator begin() { return controllers.begin(); }
    ControllerList::iterator end() { return controllers.end(); }

    friend Packer& operator<<(Packer&, HardwareManager const&);
};
