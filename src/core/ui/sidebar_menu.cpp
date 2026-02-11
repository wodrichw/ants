#include "ui/sidebar_menu.hpp"

#include "spdlog/spdlog.h"

SidebarMenu::SidebarMenu() : menu_items() {
    menu_items = {
        {"Save", SidebarMenuAction::SAVE, {}},
        {"Restore", SidebarMenuAction::RESTORE, {}},
        {"Clock Speed", SidebarMenuAction::NONE,
         {MenuItem{"Pause", SidebarMenuAction::CLOCK_PAUSE, {}},
          MenuItem{"Play", SidebarMenuAction::CLOCK_PLAY, {}},
          MenuItem{"Fast Forward", SidebarMenuAction::CLOCK_FAST_FORWARD, {}}}},
        {"Text Editor", SidebarMenuAction::TEXT_EDITOR, {}}};
}

void SidebarMenu::navigate_up() {
    if(is_submenu_open()) {
        const auto& submenu =
            menu_items[submenu_open_index.value()].children;
        if(submenu.empty()) return;
        submenu_selected = wrap_index(static_cast<long>(submenu_selected) - 1,
                                      submenu.size());
        return;
    }

    if(menu_items.empty()) return;
    selected = wrap_index(static_cast<long>(selected) - 1, menu_items.size());
}

void SidebarMenu::navigate_down() {
    if(is_submenu_open()) {
        const auto& submenu =
            menu_items[submenu_open_index.value()].children;
        if(submenu.empty()) return;
        submenu_selected = wrap_index(static_cast<long>(submenu_selected) + 1,
                                      submenu.size());
        return;
    }

    if(menu_items.empty()) return;
    selected = wrap_index(static_cast<long>(selected) + 1, menu_items.size());
}

void SidebarMenu::enter() {
    if(menu_items.empty()) return;

    if(is_submenu_open()) {
        const auto& submenu =
            menu_items[submenu_open_index.value()].children;
        if(submenu.empty()) return;
        pending_action = submenu[submenu_selected].action;
        return;
    }

    const auto& item = menu_items[selected];
    if(!item.children.empty()) {
        submenu_open_index = selected;
        submenu_selected = 0;
        return;
    }

    pending_action = item.action;
}

void SidebarMenu::back() { close_submenu(); }

std::optional<SidebarMenuAction> SidebarMenu::take_pending_action() {
    auto action = pending_action;
    pending_action = std::nullopt;
    return action;
}

void SidebarMenu::get_toggle_button_bounds(long& x, long& y, long& w, long& h) {
    w = 3;
    h = 1;
    x = globals::COLS - w;
    y = globals::ROWS - h;
}

void SidebarMenu::close_submenu() {
    submenu_open_index = std::nullopt;
    submenu_selected = 0;
}

size_t SidebarMenu::wrap_index(long next, size_t max) const {
    if(max == 0) return 0;
    long result = next % static_cast<long>(max);
    if(result < 0) result += static_cast<long>(max);
    return static_cast<size_t>(result);
}
