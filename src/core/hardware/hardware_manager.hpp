#pragma once

#include <vector>

#include <hardware/compiler.hpp>

struct ProgramExecutor;

struct HardwareManager {
    private:
    using ExecutorList = std::vector<ProgramExecutor*>;
    ExecutorList exec_list;
    Compiler compiler;

    public:
    HardwareManager(CommandMap const&);
    HardwareManager(Unpacker&, CommandMap const&);
    void push_back(ProgramExecutor*);
    void compile(MachineCode const& machine_code, AntInteractor& interactor, Status& status);

    ExecutorList::iterator begin() { return exec_list.begin(); }
    ExecutorList::iterator end() { return exec_list.end(); }

    friend Packer& operator<<(Packer&, HardwareManager const&);
};
