#pragma once

#include <vector>
#include <string>
#include <functional>
#include <unordered_set>
#include <unordered_map>



class ClockController {
public:
    using Op = std::function<void()>;
    virtual void handleClockPulse() = 0;
    virtual ~ClockController() = default;
};

struct ParserStatus {
    bool p_err;
    std::string err_msg;
    void error(const std::string& err_msg) {
        this->p_err = true;
        this->err_msg = err_msg;
    }
    ParserStatus(): p_err(false), err_msg("") {}
    ParserStatus(bool p_err, std::string err_msg): p_err(p_err), err_msg(err_msg) {}
};

struct EngineInteractor {
    using move_ant_f = std::function<void(int dx, int dy)>;

    move_ant_f move_ant;
    ParserStatus status;
    EngineInteractor() = default;
    EngineInteractor(const EngineInteractor& rhs);
};

class ParserCommandsAssembler;

class Parser {
public:
    enum Command {
        MOVE
    };

    struct CommandParser {
        size_t num_args;
        std::function<void(std::istringstream& arg_sstream)> parse;
        CommandParser(size_t num_args, std::function<void(std::istringstream& arg_sstream)> parse):
            num_args(num_args), parse(parse)
        {}
    };

    struct CommandConfig {
        std::string command_string;
        Command command_enum;
        std::function<CommandParser(EngineInteractor& interactor, std::vector<ClockController::Op>& operations)> assemble;
        CommandConfig(
            const std::string& command_string,
            Command command_enum, 
            std::function<CommandParser(EngineInteractor& interactor,
            std::vector<ClockController::Op>& operations)> assemble
        ):
            command_string(command_string),
            command_enum(command_enum),
            assemble(assemble)
        {}
    };

    struct Move {
        EngineInteractor interactor;
        int dx, dy;
        Move(const EngineInteractor& interactor, int dx, int dy);
        void operator()();
    };

public:
    std::unordered_map<std::string, CommandParser> commands;
    Parser(ParserCommandsAssembler& commands_assember,
        std::unordered_set<Command> command_set,
        EngineInteractor& interactor,
        std::vector<ClockController::Op>& operations,
        std::vector<std::string>& program_code
    );
};

class ParserCommandsAssembler {
    std::unordered_map<Parser::Command, Parser::CommandConfig*> _map;
public:
    ParserCommandsAssembler();
    const Parser::CommandConfig& operator[](Parser::Command command) { return *_map[command]; }
    void insert(Parser::CommandConfig* config) { _map[config->command_enum] = config; }
};



class  Worker_Controller: public ClockController {
public:
    struct Brain {
        int acc_reg;
        int bak_reg;
    };

    Brain* brain;

    std::vector<Op> operations;
    size_t operation_idx; // essentially program counter
    Parser parser;

    Worker_Controller(ParserCommandsAssembler& commands_assember, EngineInteractor& interactor, std::vector<std::string>& program_code);
    ~Worker_Controller();

    void handleClockPulse() override;
};
