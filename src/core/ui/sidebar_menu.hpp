#pragma once

#include <optional>
#include <string>
#include <vector>

#include "app/globals.hpp"
#include "utils/types.hpp"

enum class SidebarMenuAction {
    NONE,
    SAVE,
    RESTORE,
    TEXT_EDITOR,
    CLOCK_PAUSE,
    CLOCK_PLAY,
    CLOCK_FAST_FORWARD
};

struct MenuItem {
    std::string label;
    SidebarMenuAction action = SidebarMenuAction::NONE;
    std::vector<MenuItem> children = {};
};

class SidebarMenu {
   public:
    SidebarMenu();

    void navigate_up();
    void navigate_down();
    void enter();
    void back();

    bool is_submenu_open() const { return submenu_open_index.has_value(); }
    std::optional<size_t> open_submenu_index() const {
        return submenu_open_index;
    }
    size_t selected_index() const { return selected; }
    size_t submenu_selected_index() const { return submenu_selected; }

    const std::vector<MenuItem>& items() const { return menu_items; }

    std::optional<SidebarMenuAction> take_pending_action();

    static void get_toggle_button_bounds(long& x, long& y, long& w, long& h);

   private:
    std::vector<MenuItem> menu_items;
    size_t selected = 0;
    std::optional<size_t> submenu_open_index = std::nullopt;
    size_t submenu_selected = 0;
    std::optional<SidebarMenuAction> pending_action = std::nullopt;

    void close_submenu();
    size_t wrap_index(long next, size_t max) const;
};
