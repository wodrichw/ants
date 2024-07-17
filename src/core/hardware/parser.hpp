#pragma once

#include <string>

#include "hardware/command_config.hpp"
#include "utils/status.hpp"

struct ParserArgs;
struct ProgramExecutor;
struct MachineCode;
class LabelMap;

class Parser {
    public:

    Parser(CommandMap const&);
    void parse(std::vector<std::string> const&, MachineCode&, Status&);
    void deparse(MachineCode const&, std::vector<std::string>&, Status&);

    private:

    bool handle_label(LabelMap&, std::string const&, ushort, Status&);
    void preprocess(std::vector<std::string> const&, LabelMap&, std::vector<std::string>&, Status&);

    CommandMap const& command_map;
 };
