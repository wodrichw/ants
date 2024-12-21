#include "hardware/hardware_manager.hpp"

#include "hardware.pb.h"
#include "hardware/program_executor.hpp"
#include "utils/serializer.hpp"

HardwareManager::HardwareManager(CommandMap const& command_map)
    : compiler(command_map) {}
HardwareManager::HardwareManager(const ant_proto::HardwareManager&,
                                 CommandMap const& command_map)
    : compiler(command_map) {
    // Does not take ownership of program executor objects.
    SPDLOG_TRACE("Not unpacking empty hardware manager");
}
// Does not take ownership
void HardwareManager::push_back(ProgramExecutor* exec) {
    exec_list.push_back(exec);
}

void HardwareManager::compile(CompileArgs& args) { compiler.compile(args); }

Packer& operator<<(Packer& p, HardwareManager const&) {
    // Does not take ownership of executor objects.
    SPDLOG_TRACE("Not packing empty hardware manager");
    return p;
}
