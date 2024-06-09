#pragma once

#include <unordered_map>

#include "utils/serializer.hpp"
#include "proto/hardware.pb.h"

#include "spdlog/spdlog.h"

class LabelMap {
    std::unordered_map<std::string, ushort> label_map;
    std::unordered_map<ushort, std::string> address_map;

    public:

    LabelMap()=default;
    LabelMap(Unpacker& p) {
        ant_proto::Integer length_msg;
        p >> length_msg;

        int map_length = length_msg.value();
        SPDLOG_TRACE("Unpacking the software label map - size: {}", map_length);
        label_map.reserve(map_length);
        address_map.reserve(map_length);

        for (int i = 0; i < map_length; ++i) {
            ant_proto::LabelRecord msg;
            p >> msg;

            ushort address = msg.address();
            std::string label = msg.label();
            insert(address, label);
        }
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

    friend Packer& operator<<(Packer& p, LabelMap const& obj) {
        SPDLOG_TRACE("Packing label map - size: {}", obj.size());
        ant_proto::Integer length_msg;
        length_msg.set_value(obj.size());
        p << length_msg;

        for (auto const &[address, label]: obj.address_map) {
            ant_proto::LabelRecord msg;
            msg.set_address(address);
            msg.set_label(label);
            p << msg;
        }
        return p;
    }
};
