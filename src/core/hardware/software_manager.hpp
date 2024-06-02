#pragma once

#include <vector>

#include "hardware/command_config.hpp"
#include "hardware/machine_code.hpp"
#include "hardware/parser.hpp"
#include "spdlog/spdlog.h"

class SoftwareManager {
    Parser parser;

    std::vector<MachineCode*> code_list;
    MachineCode* current_code = new MachineCode();
    bool assigned_current = false;

   public:
    SoftwareManager(CommandMap const& command_map) : parser(command_map){};
    SoftwareManager(Unpacker& p, CommandMap const& command_map)
        : parser(command_map) {
            ant_proto::Integer count_msg;
            p >> count_msg;
            ulong code_list_length = count_msg.value();
            code_list.reserve(code_list_length);
    
            for (ulong i = 0; i < code_list_length; ++i) {
                code_list.push_back(new MachineCode(p));
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

    void assign() { assigned_current = true; }

    virtual ~SoftwareManager() {
        delete current_code;
        current_code = nullptr;

        for(MachineCode* code : code_list) delete code;
        code_list.clear();
    }

    friend Packer& operator<<(Packer& p, SoftwareManager const& obj) {
        ant_proto::Integer count_msg;
        count_msg.set_value(obj.code_list.size());
        p << count_msg;
    
        for (MachineCode const* code : obj.code_list) {
            p << (*code);
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