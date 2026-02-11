#include <gtest/gtest.h>

#include "app/globals.hpp"
#include "ui/event_system.hpp"
#include "ui/layoutbox.hpp"
#include "ui/text_editor.hpp"
#include "hardware/software_manager.hpp"
#include "hardware/command_config.hpp"

struct KeyTypeCase {
    SDL_Keycode key;
    KeyboardEventType expected;
};

class KeyboardTypeTest : public ::testing::TestWithParam<KeyTypeCase> {};

TEST_P(KeyboardTypeTest, MapsKeyboardKeys) {
    auto param = GetParam();
    EXPECT_EQ(get_keyboard_type(param.key), param.expected);
}

INSTANTIATE_TEST_SUITE_P(
    KeyboardTypeCases,
    KeyboardTypeTest,
    ::testing::Values(
        KeyTypeCase{SDLK_a, A_KEY_EVENT},
        KeyTypeCase{SDLK_z, Z_KEY_EVENT},
        KeyTypeCase{SDLK_RETURN, RETURN_KEY_EVENT},
        KeyTypeCase{SDLK_LEFT, LEFT_KEY_EVENT},
        KeyTypeCase{SDLK_UNKNOWN, UNKNOWN_KEY_EVENT}
    )
);

struct MouseTypeCase {
    char button;
    MouseEventType expected;
};

class MouseTypeTest : public ::testing::TestWithParam<MouseTypeCase> {};

TEST_P(MouseTypeTest, MapsMouseButtons) {
    auto param = GetParam();
    EXPECT_EQ(get_mouse_type(param.button), param.expected);
}

INSTANTIATE_TEST_SUITE_P(
    MouseTypeCases,
    MouseTypeTest,
    ::testing::Values(
        MouseTypeCase{SDL_BUTTON_LEFT, LEFT_MOUSE_EVENT},
        MouseTypeCase{SDL_BUTTON_RIGHT, RIGHT_MOUSE_EVENT},
        MouseTypeCase{SDL_BUTTON_MIDDLE, MIDDLE_MOUSE_EVENT}
    )
);

TEST(UiKeyboardChordTest, SetKeyFillsFirstSlot) {
    KeyboardChordEvent chord;
    chord.set_key(A_KEY_EVENT);
    EXPECT_EQ(chord.chord.first, A_KEY_EVENT);
}

TEST(UiKeyboardChordTest, SetKeyFillsSecondSlot) {
    KeyboardChordEvent chord;
    chord.set_key(A_KEY_EVENT);
    chord.set_key(B_KEY_EVENT);
    EXPECT_EQ(chord.chord.second, B_KEY_EVENT);
}

TEST(UiKeyboardChordTest, UnsetKeyResetsChord) {
    KeyboardChordEvent chord;
    chord.set_key(A_KEY_EVENT);
    chord.set_key(B_KEY_EVENT);
    chord.unset_key(A_KEY_EVENT);
    EXPECT_EQ(chord.chord.first, UNKNOWN_KEY_EVENT);
    EXPECT_EQ(chord.chord.second, UNKNOWN_KEY_EVENT);
}

TEST(UiLayoutBoxTest, SplitHorizontalCreatesChildren) {
    LayoutBox box(10, 10);
    auto& children = box.split(50, LayoutBox::Orientation::HORIZONTAL);
    EXPECT_NE(children.first, nullptr);
    EXPECT_NE(children.second, nullptr);
}

TEST(UiLayoutBoxTest, SplitVerticalCreatesChildren) {
    LayoutBox box(10, 10);
    auto& children = box.split(50, LayoutBox::Orientation::VERTICAL);
    EXPECT_NE(children.first, nullptr);
    EXPECT_NE(children.second, nullptr);
}

TEST(UiLayoutBoxTest, ResizeUpdatesDimensions) {
    LayoutBox box(1, 2, 3, 4);
    box.resize(5, 6, 7, 8);

    long x1 = 0, y1 = 0;
    box.get_abs_pos(0, 0, x1, y1);
    EXPECT_EQ(x1, 5);
    EXPECT_EQ(y1, 6);
    EXPECT_EQ(box.get_width(), 7);
    EXPECT_EQ(box.get_height(), 8);
}

TEST(UiBoxManagerTest, SidebarToggleResizesBoxes) {
    BoxManager manager(globals::COLS, globals::ROWS);

    EXPECT_FALSE(manager.is_sidebar_expanded());
    EXPECT_EQ(manager.map_box->get_width(), static_cast<long>(globals::COLS));
    EXPECT_EQ(manager.map_box->get_height(), static_cast<long>(globals::ROWS));
    EXPECT_EQ(manager.sidebar_box->get_height(), 0);

    manager.toggle_sidebar();

    EXPECT_TRUE(manager.is_sidebar_expanded());
    long expected_map_h =
        (static_cast<long>(globals::ROWS) * 80) / 100;
    long expected_sidebar_h =
        static_cast<long>(globals::ROWS) - expected_map_h;
    EXPECT_EQ(manager.map_box->get_width(), static_cast<long>(globals::COLS));
    EXPECT_EQ(manager.map_box->get_height(), expected_map_h);
    EXPECT_EQ(manager.sidebar_box->get_width(),
              static_cast<long>(globals::COLS));
    EXPECT_EQ(manager.sidebar_box->get_height(), expected_sidebar_h);

    manager.toggle_sidebar();

    EXPECT_FALSE(manager.is_sidebar_expanded());
    EXPECT_EQ(manager.map_box->get_width(), static_cast<long>(globals::COLS));
    EXPECT_EQ(manager.sidebar_box->get_height(), 0);
}

TEST(UiTextEditorTest, InsertAddsCharacter) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);

    editor.insert('a');
    EXPECT_EQ(editor.line_length(), 1);
}

TEST(UiTextEditorTest, BackspaceRemovesCharacter) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);

    editor.insert('a');
    editor.backspace();
    EXPECT_EQ(editor.line_length(), 0);
}
