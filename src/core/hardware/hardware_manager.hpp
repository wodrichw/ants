#pragma once

#include <hardware/compiler.hpp>
#include <vector>

#include "hardware.pb.h"

struct ProgramExecutor;
struct CompileArgs;

struct HardwareManager {
   private:
    using ExecutorList = std::vector<ProgramExecutor*>;
    ExecutorList exec_list;
    Compiler compiler;

   public:
    HardwareManager(CommandMap const&);
    HardwareManager(const ant_proto::HardwareManager& msg, CommandMap const&);
    void push_back(ProgramExecutor*);
    void compile(CompileArgs&);

    ExecutorList::iterator begin() { return exec_list.begin(); }
    ExecutorList::iterator end() { return exec_list.end(); }

    friend Packer& operator<<(Packer&, HardwareManager const&);
};
