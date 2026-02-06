#include "hardware/label_map.hpp"

#include <algorithm>
#include <vector>

LabelMap::LabelMap(
    const ::google::protobuf::RepeatedPtrField< ::ant_proto::LabelRecord>&
        labels_msg) {
    for(const auto& label : labels_msg) {
        insert(static_cast<ushort>(label.address()), label.label());
    }
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

bool LabelMap::contains(std::string const& label) const {
    return label_map.find(label) != label_map.end();
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
    std::vector<std::pair<ushort, std::string>> ordered;
    ordered.reserve(address_map.size());
    for(auto const& [address, label] : address_map) {
        ordered.emplace_back(address, label);
    }
    std::sort(ordered.begin(), ordered.end(),
              [](const auto& lhs, const auto& rhs) {
                  return lhs.first < rhs.first;
              });
    for(const auto& [address, label] : ordered) {
        ant_proto::LabelRecord label_record_msg;
        label_record_msg.set_address(address);
        label_record_msg.set_label(label);
        *msg.Add() = label_record_msg;
    }
    return msg;
}
