#include "hardware/brain.hpp"

#include "spdlog/spdlog.h"

DualRegisters::DualRegisters() : scent_behaviors(is_space_empty_flags) {}

DualRegisters::DualRegisters(const ant_proto::DualRegisters& msg)
    : scent_behaviors(is_space_empty_flags),
            instr_ptr_register(static_cast<ushort>(msg.instr_ptr_register())),
            base_ptr_register(static_cast<ushort>(msg.base_ptr_register())),
            stack_ptr_register(static_cast<ushort>(msg.stack_ptr_register())),
      zero_flag(msg.zero_flag()),
      instr_failed_flag(msg.instr_failed_flag()),
      dir_flag1(msg.dir_flag1()),
      dir_flag2(msg.dir_flag2()),
      is_move_flag(msg.is_move_flag()),
      is_dig_flag(msg.is_dig_flag()) {
        registers[0] = static_cast<cpu_word_size>(msg.register0());
        registers[1] = static_cast<cpu_word_size>(msg.register1());
    SPDLOG_TRACE(
        "Unpacking dual registers - registers: [{}, {}] - zero_flag: {}",
        registers[0], registers[1], zero_flag ? "ON" : "OFF");
}

cpu_word_size& DualRegisters::operator[](ulong idx) { return registers[idx]; }

cpu_word_size const& DualRegisters::operator[](ulong idx) const {
    return registers[idx];
}

ant_proto::DualRegisters DualRegisters::get_proto() const {
    ant_proto::DualRegisters msg;
    msg.set_register0(registers[0]);
    msg.set_register1(registers[1]);
    msg.set_instr_ptr_register(instr_ptr_register);
    msg.set_base_ptr_register(base_ptr_register);
    msg.set_stack_ptr_register(stack_ptr_register);
    msg.set_zero_flag(zero_flag);
    msg.set_dir_flag1(dir_flag1);
    msg.set_dir_flag2(dir_flag2);
    msg.set_instr_failed_flag(instr_failed_flag);
    msg.set_is_move_flag(is_move_flag);
    msg.set_is_dig_flag(is_dig_flag);

    return msg;
}
