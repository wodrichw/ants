#pragma once

#include <google/protobuf/map.h>
#include <google/protobuf/stubs/port.h>
#include <unordered_map>
#include "proto/entity.pb.h"
#include "utils/serializer.hpp"
using ulong = unsigned long;

// Note: ItemType enum is also defined in protobuf so messing with numbering
// will mess with the serialization
enum ItemType {
    DIRT=0, FOOD=1, EGG=2
};

struct ItemInfo {
    ItemType item;
    ulong weight = 0;
    ItemInfo(ItemType item, ulong weight): item(item), weight(weight) {}
};

class ItemInfoMap {
    std::unordered_map<ItemType, ItemInfo*> map;

    public:
    ItemInfoMap() {
        insert(DIRT, 20);
        insert(FOOD, 10);
        insert(EGG, 15);
    }

    ~ItemInfoMap() {
        for (auto& pair : map) {
            delete pair.second;
        }
    }

    ItemInfo& operator[](ItemType item) { return *(map[item]); }
    ItemInfo const& at(ItemType item) const { return *(map.at(item)); }
    ulong size() const { return map.size(); }

    private:
    void insert(ItemType item, ulong weight) {
        map[item] = new ItemInfo(item, weight);
    }
};

class Inventory {
    ulong max_stack_count = 0, stack_size = 0, max_weight = 0;
    ulong stack_count = 0, total_weight = 0;
    std::unordered_map<ItemType, ulong> items;
    ItemInfoMap const& item_info_map;

    public:

    Inventory(ulong max_stack_count, ulong stack_size, ulong max_weight, ItemInfoMap const& item_info_map): max_stack_count(max_stack_count), stack_size(stack_size), max_weight(max_weight), item_info_map(item_info_map) {
        initialize();
    }

    Inventory(const ant_proto::Inventory msg, ItemInfoMap const& item_info_map): item_info_map(item_info_map) 
    {
        initialize();

        max_stack_count = msg.max_stack_count();
        stack_size = msg.stack_size();
        max_weight = msg.max_weight();

        for(const auto& item_record: msg.item_records() ) {
            ulong count = item_record.count(); 
            add(static_cast<ItemType>(item_record.type()), count);
        }
        SPDLOG_TRACE("Completed unpacking inventory");
    }


    ulong max_space_for_item(ItemType item) {
        ulong& current_count = items[item];
        ulong current_stack_count = get_ceil_stack_count(current_count);
        ulong max_stack_count_item = current_stack_count + max_stack_count - stack_count;
        ulong stack_max_increase = max_stack_count_item * stack_size - current_count;
        ItemInfo const& info = item_info_map.at(item);
        ulong weight_max_increase = (max_weight - total_weight) / info.weight;
        return std::min(stack_max_increase, weight_max_increase);
    }

    void add(ItemType item, ulong& delta) {
        ulong& current_count = items[item];
        ulong current_stack_count = get_ceil_stack_count(current_count);
        ItemInfo const& info = item_info_map.at(item);
        ulong max_space = max_space_for_item(item);
        ulong increase_amount = std::min(max_space, delta);

        current_count += increase_amount;
        delta -= increase_amount;
        total_weight += increase_amount * info.weight;

        ulong new_stack_count = get_ceil_stack_count(current_count);
        stack_count += new_stack_count - current_stack_count;
    }

    void remove(ItemType item, ulong& delta) {
        ulong& current_count = items[item];
        ulong current_stack_count = get_ceil_stack_count(current_count);
        ulong decrease_amount = std::min(current_count, delta);

        current_count -= decrease_amount;
        delta -= decrease_amount;

        ItemInfo const& info = item_info_map.at(item);
        total_weight -= decrease_amount * info.weight;

        ulong new_stack_count = get_ceil_stack_count(current_count);
        stack_count += new_stack_count - current_stack_count;
    }

    bool has(ItemType item) const {
        return items.at(item) > 0;
    }

    ulong size() const { return items.size(); }

    ant_proto::Inventory get_proto() const {
        ant_proto::Inventory msg;

        msg.set_max_stack_count(max_stack_count);
        msg.set_stack_size(stack_size);
        msg.set_max_weight(max_weight);
        for(const auto& item: items) {
            ant_proto::InventoryRecord i_msg;
            i_msg.set_type(item.first);
            i_msg.set_count(item.second);
            *msg.add_item_records() = i_msg;
        }

        return msg;
    }

    private:

    void initialize() {
        items[DIRT] = 0;
        items[FOOD] = 0;
        items[EGG] = 0;
    }

    ulong get_ceil_stack_count(ulong count) const {
        return count / stack_size + (count % stack_size == 0 ? 0 : 1);
    }

};
