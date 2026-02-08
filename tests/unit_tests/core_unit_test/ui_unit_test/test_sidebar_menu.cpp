#include <gtest/gtest.h>

#include "app/globals.hpp"
#include "ui/sidebar_menu.hpp"

TEST(SidebarMenuTest, ConstructorBuildsExpectedItems) {
    SidebarMenu menu;
    ASSERT_EQ(menu.items().size(), 4u);
    EXPECT_EQ(menu.items()[0].label, "Save");
    EXPECT_EQ(menu.items()[0].action, SidebarMenuAction::SAVE);
    EXPECT_EQ(menu.items()[1].label, "Restore");
    EXPECT_EQ(menu.items()[1].action, SidebarMenuAction::RESTORE);
    EXPECT_EQ(menu.items()[2].label, "Clock Speed");
    EXPECT_EQ(menu.items()[2].children.size(), 3u);
    EXPECT_EQ(menu.items()[3].label, "Text Editor");
    EXPECT_EQ(menu.items()[3].action, SidebarMenuAction::TEXT_EDITOR);
}

TEST(SidebarMenuTest, NavigateWrapsInMainMenu) {
    SidebarMenu menu;
    EXPECT_EQ(menu.selected_index(), 0u);
    menu.navigate_up();
    EXPECT_EQ(menu.selected_index(), 3u);
    menu.navigate_down();
    EXPECT_EQ(menu.selected_index(), 0u);
}

TEST(SidebarMenuTest, EnterLeafSetsPendingAction) {
    SidebarMenu menu;
    menu.enter();
    auto action = menu.take_pending_action();
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(action.value(), SidebarMenuAction::SAVE);
    EXPECT_FALSE(menu.take_pending_action().has_value());
}

TEST(SidebarMenuTest, EnterParentOpensSubmenu) {
    SidebarMenu menu;
    menu.navigate_down();
    menu.navigate_down();
    menu.enter();
    EXPECT_TRUE(menu.is_submenu_open());
    ASSERT_TRUE(menu.open_submenu_index().has_value());
    EXPECT_EQ(menu.open_submenu_index().value(), 2u);
    EXPECT_EQ(menu.submenu_selected_index(), 0u);
}

TEST(SidebarMenuTest, SubmenuNavigationAndBack) {
    SidebarMenu menu;
    menu.navigate_down();
    menu.navigate_down();
    menu.enter();

    menu.navigate_down();
    EXPECT_EQ(menu.submenu_selected_index(), 1u);
    menu.navigate_down();
    menu.navigate_down();
    EXPECT_EQ(menu.submenu_selected_index(), 0u);

    menu.back();
    EXPECT_FALSE(menu.is_submenu_open());
    EXPECT_EQ(menu.submenu_selected_index(), 0u);
}

TEST(SidebarMenuTest, SubmenuEnterSetsPendingAction) {
    SidebarMenu menu;
    menu.navigate_down();
    menu.navigate_down();
    menu.enter();
    menu.navigate_down();
    menu.enter();

    auto action = menu.take_pending_action();
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(action.value(), SidebarMenuAction::CLOCK_PLAY);
}

TEST(SidebarMenuTest, ToggleButtonBoundsMatchGlobals) {
    long x = 0, y = 0, w = 0, h = 0;
    SidebarMenu::get_toggle_button_bounds(x, y, w, h);
    EXPECT_EQ(w, 3);
    EXPECT_EQ(h, 1);
    EXPECT_EQ(x, static_cast<long>(globals::COLS) - w);
    EXPECT_EQ(y, 0);
}