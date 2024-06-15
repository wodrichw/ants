#include "hardware/hardware_manager.hpp"
#include "utils/serializer.hpp"
#include "hardware/program_executor.hpp"

HardwareManager::HardwareManager(CommandMap const& command_map): compiler(command_map) {}
HardwareManager::HardwareManager(Unpacker&, CommandMap const& command_map): compiler(command_map) {
    // Does not take ownership of program executor objects.
    SPDLOG_TRACE("Not unpacking empty hardware manager");
}
// Does not take ownership
void HardwareManager::push_back(ProgramExecutor* exec) { exec_list.push_back(exec); }

void HardwareManager::compile(MachineCode const& machine_code, AntInteractor& interactor, Status& status) { compiler.compile(machine_code, interactor, status); }

Packer& operator<<(Packer& p, HardwareManager const&) {
    // Does not take ownership of executor objects.
    SPDLOG_TRACE("Not packing empty hardware manager");
    return p;
}
