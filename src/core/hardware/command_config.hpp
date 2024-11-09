#pragma once

#include <string>
#include <functional>

struct ParseArgs;
struct DeparseArgs;
struct CompileArgs;

enum CommandEnum {
    NOP=0b00000, MOVE=0b00001, LOAD=0b00010, COPY=0b00011, ADD=0b00100,
    SUB=0b00101, INC=0b00110, DEC=0b00111, PUSH=0b01000, POP=0b01001,
    JMP=0b01010, JNZ=0b01011, CALL=0b01100, RET=0b01101, JNF=0b01110,
    LT=0b01111, RT=0b10000, DIG=0b10001, CHECK=0b10010, SCENT_ON=0b10011,
    SCENT_OFF=0b10100, SET_SCENT_PRIORITY=0b10101, TURN_SCENT=0b10110 };

struct CommandConfig {
    std::string command_string;
    CommandEnum command_enum;
    std::function<void(CommandConfig const&, ParseArgs&)> parse;
    std::function<void(CommandConfig const&, DeparseArgs&)> deparse;
    std::function<void(CommandConfig const&, CompileArgs&)> compile;

    CommandConfig(const std::string &command_string, CommandEnum command_enum,
        std::function<void(CommandConfig const&, ParseArgs&)> parse,
        std::function<void(CommandConfig const&, DeparseArgs&)> deparse,
        std::function<void(CommandConfig const&, CompileArgs&)> compile);
};

class CommandMap {
    using EnumMap = std::unordered_map<CommandEnum, CommandConfig*>;
    using StrMap = std::unordered_map<std::string, CommandConfig*>;
    EnumMap enum_map;
    StrMap str_map;

    public:
    CommandMap();
    virtual ~CommandMap();
    void insert(CommandConfig*);
    CommandConfig& operator[](CommandEnum);
    CommandConfig& operator[](std::string const&);
    CommandConfig const& at(CommandEnum) const;
    CommandConfig const& at(std::string const&) const;

    EnumMap::const_iterator find(CommandEnum) const;
    EnumMap::const_iterator enum_end() const;
    StrMap::const_iterator find(std::string const&) const;
    StrMap::const_iterator str_end() const;
};
