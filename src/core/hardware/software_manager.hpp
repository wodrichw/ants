#pragma once

#include <vector>

#include "hardware/command_config.hpp"
#include "hardware/machine_code.hpp"
#include "hardware/parser.hpp"
#include "spdlog/spdlog.h"

class SoftwareManager {
    Parser parser;

    std::vector<MachineCode*> code_list;
    std::unordered_map<ulong, ulong> ant_mapping;
    MachineCode* current_code = new MachineCode();
    bool assigned_current = false;

   public:
    SoftwareManager(CommandMap const& command_map) : parser(command_map){};
    SoftwareManager(Unpacker& p, CommandMap const& command_map, ulong worker_count)
        : parser(command_map), current_code(new MachineCode(p)) {
            ant_proto::SoftwareManager sftmgr_msg;
            p >> sftmgr_msg;
            ulong code_list_length = sftmgr_msg.code_list_length();
            code_list.reserve(code_list_length);
            assigned_current = sftmgr_msg.assigned_current();
    
            SPDLOG_DEBUG("Unpacking code list - count: {}", code_list_length);
            for (ulong i = 0; i < code_list_length; ++i) {
                code_list.push_back(new MachineCode(p));
            }

            SPDLOG_DEBUG("Unpacking worker/code mappings - count: {}", worker_count);
            for (ulong i = 0; i < worker_count; ++i) {
               ant_proto::AntCodeRecord msg;
               p >> msg;

               ant_mapping[msg.ant_idx()] = msg.code_idx(); 
               SPDLOG_TRACE("Worker index: {} - code length: {} [index: {}]", msg.ant_idx(), (*this)[msg.ant_idx()].size(), msg.code_idx());
            }
        }

    bool has_code() const { return !current_code->is_empty(); }

    void add_lines(std::vector<std::string> const& lines) {
        SPDLOG_DEBUG("Getting the instruction strings from the machine code");
        clear_current();

        Status status;
        parser.parse(lines, *current_code, status);

        if(status.p_err) {
            SPDLOG_ERROR("Failed to parse program - clearing machine code...");
            current_code->clear();
        }
    }

    void get_lines(std::vector<std::string>& lines) {
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

    MachineCode& get() { return *current_code; }

    MachineCode& operator[](ulong ant_idx) {
        ulong code_idx = ant_mapping[ant_idx];
        return code_idx >= code_list.size() ? *current_code : *(code_list[code_idx]); 
    }

    void assign(ulong ant_idx) {
        assigned_current = true;

        // needs to be called before current is added to the list
        ant_mapping[ant_idx] = code_list.size();
    }

    virtual ~SoftwareManager() {
        delete current_code;
        current_code = nullptr;

        for(MachineCode* code : code_list) delete code;
        code_list.clear();
    }

    friend Packer& operator<<(Packer& p, SoftwareManager const& obj) {
        SPDLOG_DEBUG("Packing software manager - assigned current: {}", obj.assigned_current);
        ant_proto::SoftwareManager sftmgr_msg;
        sftmgr_msg.set_code_list_length(obj.code_list.size());
        sftmgr_msg.set_assigned_current(obj.assigned_current);

        SPDLOG_DEBUG("Packing current code");
        p << (*obj.current_code) << sftmgr_msg;
    
        SPDLOG_DEBUG("Packing code list - count: {}", obj.code_list.size());
        for (MachineCode const* code : obj.code_list) {
            p << (*code);
        }

        SPDLOG_DEBUG("Packing machine code ant mapping - size: {}", obj.ant_mapping.size());
        for (auto const& [ant_idx, code_idx] : obj.ant_mapping) {
            ant_proto::AntCodeRecord msg;
            msg.set_ant_idx(ant_idx);
            msg.set_code_idx(code_idx);
            p << msg;
        }
        return p;
     }

   private:
    void clear_current() {
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
};