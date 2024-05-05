#pragma once

#include <unordered_map>

class LabelMap {
    std::unordered_map<std::string, ushort> label_map;
    std::unordered_map<ushort, std::string> address_map;

    public:

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
};
