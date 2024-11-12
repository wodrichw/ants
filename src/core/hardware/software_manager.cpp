#include "hardware/software_manager.hpp"
#include "spdlog/spdlog.h"

SoftwareManager::SoftwareManager(const ant_proto::SoftwareManager& msg, CommandMap const& command_map):
    parser(command_map),
    current_code(new MachineCode(msg.current_code())),
    assigned_current(msg.assigned_current())
{
    for( const auto& ant_machine_code: msg.ant_machine_codes() )
        code_list.emplace_back(new MachineCode(ant_machine_code));

    for( const auto& ant_code_record: msg.ant_code_records() )
        ant_mapping[ant_code_record.ant_idx()] = ant_code_record.code_idx(); 
}

bool SoftwareManager::has_code() const {
    return !current_code->is_empty();
}

void SoftwareManager::add_lines(std::vector<std::string> const& lines) {
    SPDLOG_DEBUG("Getting the instruction strings from the machine code");
    clear_current();

    Status status;
    parser.parse(lines, *current_code, status);

    if(status.p_err) {
        SPDLOG_ERROR("Failed to parse program - clearing machine code...");
        current_code->clear();
    }
}

void SoftwareManager::get_lines(std::vector<std::string>& lines) {
    Status status;
    lines.clear();
    parser.deparse(*current_code, lines, status);
    lines.push_back("");

    if(status.p_err) {
        SPDLOG_ERROR(
            "Failed to deparse program - clearing string lines...");
        lines.clear();
    }
}

MachineCode& SoftwareManager::get() {
    return *current_code;
}

MachineCode& SoftwareManager::operator[](ulong ant_idx) {
    ulong code_idx = ant_mapping[ant_idx];
    return code_idx >= code_list.size() ? *current_code : *(code_list[code_idx]); 
}

void SoftwareManager::assign(ulong ant_idx) {
    assigned_current = true;

    // needs to be called before current is added to the list
    ant_mapping[ant_idx] = code_list.size();
}

SoftwareManager::~SoftwareManager() {
    delete current_code;
    current_code = nullptr;

    for(MachineCode* code : code_list) delete code;
    code_list.clear();
}

ant_proto::SoftwareManager SoftwareManager::get_proto() const {
    ant_proto::SoftwareManager msg;
    msg.set_assigned_current(assigned_current);
    *msg.mutable_current_code() = current_code->get_proto();

    const volatile std::string current_code_str = msg.current_code().code();
    for (const auto& code : code_list)
        *msg.add_ant_machine_codes() = code->get_proto();

    for (auto const& [ant_idx, code_idx] : ant_mapping) {
        ant_proto::AntCodeRecord ant_code_record_msg;
        ant_code_record_msg.set_ant_idx(ant_idx);
        ant_code_record_msg.set_code_idx(code_idx);
        *msg.add_ant_code_records() = ant_code_record_msg;
    }
    return msg;
}

void SoftwareManager::clear_current() {
    if(!assigned_current) {
        SPDLOG_DEBUG(
            "No workers were assigned the code - clearing the code: {} "
            "instructions",
            current_code->size());
        current_code->clear();
        return;
    }

    SPDLOG_DEBUG("Worker was assigned the code - storing the machine code");
    code_list.push_back(current_code);

    SPDLOG_TRACE("Resetting the software manager current code context");
    current_code = new MachineCode();
    assigned_current = false;
    SPDLOG_TRACE("Succesfully reset the current code context");
}

