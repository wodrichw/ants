#pragma once

#include <google/protobuf/map.h>
#include <google/protobuf/stubs/port.h>

#include <unordered_map>

#include "proto/entity.pb.h"
#include "utils/serializer.hpp"
using ulong = unsigned long;

// Note: ItemType enum is also defined in protobuf so messing with numbering
// will mess with the serialization
enum ItemType { DIRT = 0, FOOD = 1, EGG = 2 };

struct ItemInfo {
    ItemType item;
    ulong weight = 0;
    ItemInfo(ItemType item, ulong weight) : item(item), weight(weight) {}
};

class ItemInfoMap {
    std::unordered_map<ItemType, ItemInfo*> map;

   public:
    ItemInfoMap();
    ~ItemInfoMap();

    ItemInfo& operator[](ItemType item);
    ItemInfo const& at(ItemType item) const;
    ulong size() const;

   private:
    void insert(ItemType item, ulong weight);
};

class Inventory {
    ulong max_stack_count = 0, stack_size = 0, max_weight = 0;
    ulong stack_count = 0, total_weight = 0;
    std::unordered_map<ItemType, ulong> items;
    ItemInfoMap const& item_info_map;

   public:
    Inventory(ulong max_stack_count, ulong stack_size, ulong max_weight,
              ItemInfoMap const& item_info_map);
    Inventory(const ant_proto::Inventory msg, ItemInfoMap const& item_info_map);

    ulong max_space_for_item(ItemType item);
    void add(ItemType item, ulong& delta);
    void remove(ItemType item, ulong& delta);
    bool has(ItemType item) const;
    ulong size() const;
    ant_proto::Inventory get_proto() const;

   private:
    void initialize();
    ulong get_ceil_stack_count(ulong count) const;
};
