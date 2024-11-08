#pragma once

#include <unordered_map>

#include "proto/hardware.pb.h"

class LabelMap {
    std::unordered_map<std::string, ushort> label_map;
    std::unordered_map<ushort, std::string> address_map;

    public:

    LabelMap()=default;
    LabelMap( const ::google::protobuf::RepeatedPtrField< ::ant_proto::LabelRecord> & labels_msg) {
        for( const auto& label: labels_msg ) 
            insert(label.address(), label.label());
    }

    void insert(ushort address, std::string const& label) {
        label_map[label] = address;
        address_map[address] = label;
    }

    std::string const& at(ushort address) const {
        return address_map.at(address);
    }

    ushort at(std::string const& label) const {
        return label_map.at(label);
    }

    inline size_t size() const {
        return label_map.size();
    }

    void clear() {
        label_map.clear();
        address_map.clear();
    }

    void get_addresses(std::vector<ushort>& out) const {
        for( auto const& pair: label_map) {
            out.push_back(pair.second);
        }
    }

    google::protobuf::RepeatedPtrField< ant_proto::LabelRecord> get_proto() const {
        google::protobuf::RepeatedPtrField< ant_proto::LabelRecord> msg;
        for (auto const &[address, label]: address_map) {
            ant_proto::LabelRecord label_record_msg;
            label_record_msg.set_address(address);
            label_record_msg.set_label(label);
            *msg.Add() = label_record_msg;
        }
        return msg;
    }
};

