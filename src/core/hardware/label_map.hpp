#pragma once

#include <unordered_map>

#include "app/globals.hpp"
#include "proto/hardware.pb.h"
#include "utils/types.hpp"

class LabelMap {
    std::unordered_map<std::string, ushort> label_map;
    std::unordered_map<ushort, std::string> address_map;

   public:
    LabelMap() = default;
    LabelMap(
        const ::google::protobuf::RepeatedPtrField< ::ant_proto::LabelRecord>&
            labels_msg);

    void insert(ushort address, std::string const& label);
    std::string const& at(ushort address) const;
    ushort at(std::string const& label) const;
    bool contains(std::string const& label) const;
    inline ulong size() const {
        return static_cast<ulong>(label_map.size());
    }
    void clear();
    void get_addresses(std::vector<ushort>& out) const;
    google::protobuf::RepeatedPtrField<ant_proto::LabelRecord> get_proto()
        const;
};
