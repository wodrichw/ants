#include "hardware/label_map.hpp"

using ushort = unsigned short;

LabelMap::LabelMap(
    const ::google::protobuf::RepeatedPtrField< ::ant_proto::LabelRecord>&
        labels_msg) {
    for(const auto& label : labels_msg) insert(label.address(), label.label());
}

void LabelMap::insert(ushort address, std::string const& label) {
    label_map[label] = address;
    address_map[address] = label;
}

std::string const& LabelMap::at(ushort address) const {
    return address_map.at(address);
}

ushort LabelMap::at(std::string const& label) const {
    return label_map.at(label);
}

void LabelMap::clear() {
    label_map.clear();
    address_map.clear();
}

void LabelMap::get_addresses(std::vector<ushort>& out) const {
    for(auto const& pair : label_map) {
        out.push_back(pair.second);
    }
}

google::protobuf::RepeatedPtrField<ant_proto::LabelRecord> LabelMap::get_proto()
    const {
    google::protobuf::RepeatedPtrField<ant_proto::LabelRecord> msg;
    for(auto const& [address, label] : address_map) {
        ant_proto::LabelRecord label_record_msg;
        label_record_msg.set_address(address);
        label_record_msg.set_label(label);
        *msg.Add() = label_record_msg;
    }
    return msg;
}
