#include "hardware/hardware_manager.hpp"
#include "utils/serializer.hpp"
#include "hardware/controller.hpp"
#include "hardware/program_executor.hpp"

HardwareManager::HardwareManager(CommandMap const& command_map): compiler(command_map) {}
HardwareManager::HardwareManager(Unpacker&, CommandMap const& command_map): compiler(command_map) {
    // Does not take ownership of clock controller objects.
}
// Does not take ownership
void HardwareManager::push_back(ClockController* controller) { controllers.push_back(controller); }

void HardwareManager::compile(MachineCode const& machine_code, AntInteractor& interactor, Status& status) { compiler.compile(machine_code, interactor, status); }

Packer& operator<<(Packer& p, HardwareManager const&) {
    // Does not take ownership of clock controller objects.
    return p;
}
