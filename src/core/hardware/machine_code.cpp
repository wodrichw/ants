#include "hardware/machine_code.hpp"
#include "hardware/label_map.hpp"

MachineCode::MachineCode(const ant_proto::MachineCode& msg) : labels(msg.labels()) {
    for( const auto& byte: msg.code() )
        code.emplace_back(byte);
}

void MachineCode::clear() {
    labels.clear();
    code.clear();
}

bool MachineCode::is_empty() const {
    return code.size() == 0 && labels.size() == 0;
}

size_t MachineCode::size() const {
    return code.size();
}

ant_proto::MachineCode MachineCode::get_proto() {
    ant_proto::MachineCode msg;
    *msg.mutable_labels() = labels.get_proto();
    std::string str_code;
    str_code.append(code.begin(), code.end());
    *msg.mutable_code() = str_code;
    return msg;
}

