#pragma once

#include <vector>

#include "hardware.pb.h"
#include "hardware/command_config.hpp"
#include "hardware/machine_code.hpp"
#include "hardware/parser.hpp"

class SoftwareManager {
    Parser parser;

    std::vector<MachineCode*> code_list;
    std::unordered_map<ulong, ulong> ant_mapping;
    MachineCode* current_code = new MachineCode();
    bool assigned_current = false;

   public:
    SoftwareManager(CommandMap const& command_map) : parser(command_map){};
    SoftwareManager(const ant_proto::SoftwareManager& msg, CommandMap const& command_map);
    virtual ~SoftwareManager();
    ant_proto::SoftwareManager get_proto() const;

    bool has_code() const;
    void add_lines(std::vector<std::string> const& lines);
    void get_lines(std::vector<std::string>& lines);
    MachineCode& get();
    MachineCode& operator[](ulong ant_idx);
    void assign(ulong ant_idx);

   private:
    void clear_current();
};
