#include <gtest/gtest.h>

#include <SDL2/SDL_keycode.h>

#include <cctype>
#include <vector>

#include "hardware/command_config.hpp"
#include "hardware/software_manager.hpp"
#include "ui/event_system.hpp"
#include "ui/text_editor.hpp"
#include "ui/text_editor_handler.hpp"
#include "ui/ui_handlers.hpp"

template <typename Event>
struct CountingSubscriber : public Subscriber<Event> {
    int call_count = 0;
    Event last_event = {};

    void operator()(Event const& event) override {
        ++call_count;
        last_event = event;
    }
};

void reset_editor(TextEditor& editor) {
    editor.reset();
    editor.go_to_text_y();
    editor.move_to_start_line();
}

struct KeyTypeFull {
    SDL_Keycode key;
    KeyboardEventType expected;
};

class KeyboardTypeFullTest : public ::testing::TestWithParam<KeyTypeFull> {};

TEST_P(KeyboardTypeFullTest, MapsKey) {
    EXPECT_EQ(get_keyboard_type(GetParam().key), GetParam().expected);
}

INSTANTIATE_TEST_SUITE_P(
    AllKeys,
    KeyboardTypeFullTest,
    ::testing::Values(
        KeyTypeFull{SDLK_a, A_KEY_EVENT},
        KeyTypeFull{SDLK_b, B_KEY_EVENT},
        KeyTypeFull{SDLK_c, C_KEY_EVENT},
        KeyTypeFull{SDLK_d, D_KEY_EVENT},
        KeyTypeFull{SDLK_e, E_KEY_EVENT},
        KeyTypeFull{SDLK_f, F_KEY_EVENT},
        KeyTypeFull{SDLK_g, G_KEY_EVENT},
        KeyTypeFull{SDLK_h, H_KEY_EVENT},
        KeyTypeFull{SDLK_i, I_KEY_EVENT},
        KeyTypeFull{SDLK_j, J_KEY_EVENT},
        KeyTypeFull{SDLK_k, K_KEY_EVENT},
        KeyTypeFull{SDLK_l, L_KEY_EVENT},
        KeyTypeFull{SDLK_m, M_KEY_EVENT},
        KeyTypeFull{SDLK_n, N_KEY_EVENT},
        KeyTypeFull{SDLK_o, O_KEY_EVENT},
        KeyTypeFull{SDLK_p, P_KEY_EVENT},
        KeyTypeFull{SDLK_q, Q_KEY_EVENT},
        KeyTypeFull{SDLK_r, R_KEY_EVENT},
        KeyTypeFull{SDLK_s, S_KEY_EVENT},
        KeyTypeFull{SDLK_t, T_KEY_EVENT},
        KeyTypeFull{SDLK_u, U_KEY_EVENT},
        KeyTypeFull{SDLK_v, V_KEY_EVENT},
        KeyTypeFull{SDLK_w, W_KEY_EVENT},
        KeyTypeFull{SDLK_x, X_KEY_EVENT},
        KeyTypeFull{SDLK_y, Y_KEY_EVENT},
        KeyTypeFull{SDLK_z, Z_KEY_EVENT},
        KeyTypeFull{SDLK_0, ZERO_KEY_EVENT},
        KeyTypeFull{SDLK_1, ONE_KEY_EVENT},
        KeyTypeFull{SDLK_2, TWO_KEY_EVENT},
        KeyTypeFull{SDLK_3, THREE_KEY_EVENT},
        KeyTypeFull{SDLK_4, FOUR_KEY_EVENT},
        KeyTypeFull{SDLK_5, FIVE_KEY_EVENT},
        KeyTypeFull{SDLK_6, SIX_KEY_EVENT},
        KeyTypeFull{SDLK_7, SEVEN_KEY_EVENT},
        KeyTypeFull{SDLK_8, EIGHT_KEY_EVENT},
        KeyTypeFull{SDLK_9, NINE_KEY_EVENT},
        KeyTypeFull{SDLK_LEFT, LEFT_KEY_EVENT},
        KeyTypeFull{SDLK_RIGHT, RIGHT_KEY_EVENT},
        KeyTypeFull{SDLK_UP, UP_KEY_EVENT},
        KeyTypeFull{SDLK_DOWN, DOWN_KEY_EVENT},
        KeyTypeFull{SDLK_RETURN, RETURN_KEY_EVENT},
        KeyTypeFull{SDLK_BACKSPACE, BACKSPACE_KEY_EVENT},
        KeyTypeFull{SDLK_SPACE, SPACE_KEY_EVENT},
        KeyTypeFull{SDLK_COLON, COLON_KEY_EVENT},
        KeyTypeFull{SDLK_SLASH, SLASH_KEY_EVENT},
        KeyTypeFull{SDLK_KP_DIVIDE, SLASH_KEY_EVENT},
        KeyTypeFull{SDLK_BACKSLASH, BACK_SLASH_KEY_EVENT},
        KeyTypeFull{SDLK_F1, UNKNOWN_KEY_EVENT},
        KeyTypeFull{SDLK_UNKNOWN, UNKNOWN_KEY_EVENT}
    )
);

TEST(GetKeyboardKeyTest, KpDivideMapToSlash) {
    SDL_Keysym sym = {};
    sym.sym = SDLK_KP_DIVIDE;
    sym.mod = 0;
    EXPECT_EQ(get_keyboard_key(sym), SDLK_SLASH);
}

TEST(GetKeyboardKeyTest, ThreeWithShiftMapsToHash) {
    SDL_Keysym sym = {};
    sym.sym = SDLK_3;
    sym.mod = KMOD_LSHIFT;
    EXPECT_EQ(get_keyboard_key(sym), SDLK_HASH);
}

TEST(GetKeyboardKeyTest, SemicolonWithShiftMapsToColon) {
    SDL_Keysym sym = {};
    sym.sym = SDLK_SEMICOLON;
    sym.mod = KMOD_RSHIFT;
    EXPECT_EQ(get_keyboard_key(sym), SDLK_COLON);
}

TEST(GetKeyboardKeyTest, UnshiftedPassthrough) {
    SDL_Keysym sym = {};
    sym.sym = SDLK_a;
    sym.mod = 0;
    EXPECT_EQ(get_keyboard_key(sym), SDLK_a);
}

TEST(GetKeyboardKeyTest, ShiftedLetterPassthrough) {
    SDL_Keysym sym = {};
    sym.sym = SDLK_a;
    sym.mod = KMOD_LSHIFT;
    EXPECT_EQ(get_keyboard_key(sym), SDLK_a);
}

TEST(SetCharKeyboardTypeTest, UnshiftedLetter) {
    SDL_Keysym sym = {};
    sym.sym = SDLK_a;
    sym.mod = 0;
    CharKeyboardEvent ev;
    set_char_keyboard_type(sym, ev);
    EXPECT_EQ(ev.key, 'a');
}

TEST(SetCharKeyboardTypeTest, ShiftedThreeGivesHash) {
    SDL_Keysym sym = {};
    sym.sym = SDLK_3;
    sym.mod = KMOD_LSHIFT;
    CharKeyboardEvent ev;
    set_char_keyboard_type(sym, ev);
    EXPECT_EQ(ev.key, '#');
}

TEST(SetCharKeyboardTypeTest, ShiftedSemicolonGivesColon) {
    SDL_Keysym sym = {};
    sym.sym = SDLK_SEMICOLON;
    sym.mod = KMOD_LSHIFT;
    CharKeyboardEvent ev;
    set_char_keyboard_type(sym, ev);
    EXPECT_EQ(ev.key, ':');
}

TEST(SetCharKeyboardTypeTest, KpDivideGivesSlash) {
    SDL_Keysym sym = {};
    sym.sym = SDLK_KP_DIVIDE;
    sym.mod = 0;
    CharKeyboardEvent ev;
    set_char_keyboard_type(sym, ev);
    EXPECT_EQ(ev.key, '/');
}

TEST(EventPublisherTest, NotifyDeliversToSubscriber) {
    EventPublisher<KeyboardEventType, KeyboardEvent> pub;
    CountingSubscriber<KeyboardEvent> sub;
    pub.add(A_KEY_EVENT, &sub);

    KeyboardEvent event;
    event.type = A_KEY_EVENT;
    pub.notify(event);
    EXPECT_EQ(sub.call_count, 1);
}

TEST(EventPublisherTest, NotifyIgnoresUnregisteredKey) {
    EventPublisher<KeyboardEventType, KeyboardEvent> pub;
    CountingSubscriber<KeyboardEvent> sub;
    pub.add(A_KEY_EVENT, &sub);

    KeyboardEvent event;
    event.type = B_KEY_EVENT;
    pub.notify(event);
    EXPECT_EQ(sub.call_count, 0);
}

TEST(EventPublisherTest, MultipleSubscribersOnSameKey) {
    EventPublisher<KeyboardEventType, KeyboardEvent> pub;
    CountingSubscriber<KeyboardEvent> sub1;
    CountingSubscriber<KeyboardEvent> sub2;
    pub.add(A_KEY_EVENT, &sub1);
    pub.add(A_KEY_EVENT, &sub2);

    KeyboardEvent event;
    event.type = A_KEY_EVENT;
    pub.notify(event);
    EXPECT_EQ(sub1.call_count, 1);
    EXPECT_EQ(sub2.call_count, 1);
}

TEST(EventPublisherTest, RemoveSubscriberStopsDelivery) {
    EventPublisher<KeyboardEventType, KeyboardEvent> pub;
    CountingSubscriber<KeyboardEvent> sub;
    pub.add(A_KEY_EVENT, &sub);
    pub.remove(A_KEY_EVENT, &sub);

    KeyboardEvent event;
    event.type = A_KEY_EVENT;
    pub.notify(event);
    EXPECT_EQ(sub.call_count, 0);
}

TEST(EventPublisherTest, ChordPublisherRoutesCorrectly) {
    EventPublisher<KeyboardChordEventType, KeyboardChordEvent> pub;
    CountingSubscriber<KeyboardChordEvent> sub;
    pub.add({D_KEY_EVENT, LEFT_KEY_EVENT}, &sub);

    KeyboardChordEvent match;
    match.chord = {D_KEY_EVENT, LEFT_KEY_EVENT};
    pub.notify(match);
    EXPECT_EQ(sub.call_count, 1);

    KeyboardChordEvent no_match;
    no_match.chord = {D_KEY_EVENT, RIGHT_KEY_EVENT};
    pub.notify(no_match);
    EXPECT_EQ(sub.call_count, 1);
}

TEST(EventPublisherTest, CharPublisherDelivers) {
    EventPublisher<CharKeyboardEventType, CharKeyboardEvent> pub;
    CountingSubscriber<CharKeyboardEvent> sub;
    pub.add(CHAR_KEY_EVENT, &sub);

    CharKeyboardEvent event;
    event.key = 'x';
    pub.notify(event);
    EXPECT_EQ(sub.call_count, 1);
    EXPECT_EQ(sub.last_event.key, 'x');
}

TEST(ChordStateMachineTest, UnsetSecondKeyOnlyResetsSecond) {
    KeyboardChordEvent chord;
    chord.set_key(A_KEY_EVENT);
    chord.set_key(B_KEY_EVENT);
    chord.unset_key(B_KEY_EVENT);
    EXPECT_EQ(chord.chord.first, A_KEY_EVENT);
    EXPECT_EQ(chord.chord.second, UNKNOWN_KEY_EVENT);
}

TEST(ChordStateMachineTest, UnsetNonMatchingKeyNoChange) {
    KeyboardChordEvent chord;
    chord.set_key(A_KEY_EVENT);
    chord.set_key(B_KEY_EVENT);
    chord.unset_key(C_KEY_EVENT);
    EXPECT_EQ(chord.chord.first, A_KEY_EVENT);
    EXPECT_EQ(chord.chord.second, UNKNOWN_KEY_EVENT);
}

TEST(ChordStateMachineTest, SetThirdKeyOverwritesSecond) {
    KeyboardChordEvent chord;
    chord.set_key(A_KEY_EVENT);
    chord.set_key(B_KEY_EVENT);
    chord.set_key(C_KEY_EVENT);
    EXPECT_EQ(chord.chord.second, C_KEY_EVENT);
}

TEST(ChordStateMachineTest, EmptyChordIsUnknown) {
    KeyboardChordEvent chord;
    EXPECT_EQ(chord.chord.first, UNKNOWN_KEY_EVENT);
    EXPECT_EQ(chord.chord.second, UNKNOWN_KEY_EVENT);
}

TEST(ReloadGameHandlerTest, SetsReloadFlag) {
    bool reload = false;
    ReloadGameHandler handler(reload);
    KeyboardEvent event;
    handler(event);
    EXPECT_TRUE(reload);
}

namespace {

class UnitTestRenderer : public Renderer {
   public:
    enum class RenderMode { DEFAULT, SCENT };

    void render_map(LayoutBox const&, Map&, MapWindow const&) override {}
    void render_ant(LayoutBox const&, Map&, EntityData&,
                    MapWindow const&) override {}
    void render_building(LayoutBox const&, Building&,
                         MapWindow const&) override {}
    void render_text_editor(LayoutBox const&, TextEditor const&,
                            ulong) override {}
    void render_help_boxes(LayoutBox const&) override {}
    void present() override {}
    void pixel_to_tile_coordinates(int, int, long& tile_x,
                                   long& tile_y) override {
        tile_x = 0;
        tile_y = 0;
    }
    void use_default_tile_rendering() override {
        mode = RenderMode::DEFAULT;
        scent_idx = 0;
    }
    void use_scent_tile_rendering(ulong idx) override {
        mode = RenderMode::SCENT;
        scent_idx = idx;
    }

    RenderMode get_render_mode() const { return mode; }
    ulong get_scent_index() const { return scent_idx; }

   private:
    RenderMode mode = RenderMode::DEFAULT;
    ulong scent_idx = 0;
};

}  // namespace

TEST(DefaultMapTileRendererHandlerTest, SetsDefaultMode) {
    UnitTestRenderer renderer;
    renderer.use_scent_tile_rendering(3);
    DefaultMapTileRendererHandler handler(renderer);
    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(renderer.get_render_mode(),
              UnitTestRenderer::RenderMode::DEFAULT);
}

struct ScentCase {
    KeyboardEventType key_type;
    ulong expected_index;
};

class ScentRendererHandlerTest
    : public ::testing::TestWithParam<ScentCase> {};

TEST_P(ScentRendererHandlerTest, SetsScentMode) {
    UnitTestRenderer renderer;
    ScentMapTileRendererHandler handler(renderer);
    KeyboardEvent event;
    event.type = GetParam().key_type;
    handler(event);
    EXPECT_EQ(renderer.get_render_mode(),
              UnitTestRenderer::RenderMode::SCENT);
    EXPECT_EQ(renderer.get_scent_index(), GetParam().expected_index);
}

INSTANTIATE_TEST_SUITE_P(
    AllScents,
    ScentRendererHandlerTest,
    ::testing::Values(
        ScentCase{ONE_KEY_EVENT, 0},
        ScentCase{TWO_KEY_EVENT, 1},
        ScentCase{THREE_KEY_EVENT, 2},
        ScentCase{FOUR_KEY_EVENT, 3},
        ScentCase{FIVE_KEY_EVENT, 4},
        ScentCase{SIX_KEY_EVENT, 5},
        ScentCase{SEVEN_KEY_EVENT, 6},
        ScentCase{EIGHT_KEY_EVENT, 7}
    )
);

TEST(EditorKeyHandlerTest, AcceptsLowercaseLetters) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    EditorKeyHandler handler(editor);

    for(char ch = 'a'; ch <= 'z'; ++ch) {
        reset_editor(editor);
        CharKeyboardEvent event;
        event.key = ch;
        handler(event);
        ASSERT_EQ(editor.lines[0].size(), 1u)
            << "Failed for char: " << ch;
        EXPECT_EQ(editor.lines[0][0],
                  static_cast<char>(std::toupper(
                      static_cast<unsigned char>(ch))));
    }
}

TEST(EditorKeyHandlerTest, AcceptsDigits) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    EditorKeyHandler handler(editor);

    for(char ch = '0'; ch <= '9'; ++ch) {
        reset_editor(editor);
        CharKeyboardEvent event;
        event.key = ch;
        handler(event);
        ASSERT_EQ(editor.lines[0].size(), 1u)
            << "Failed for digit: " << ch;
        EXPECT_EQ(editor.lines[0][0], ch);
    }
}

TEST(EditorKeyHandlerTest, AcceptsSpecialChars) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    EditorKeyHandler handler(editor);

    const std::vector<char> specials = {',', ' ', '#', ':', '/', '-'};
    for(char ch : specials) {
        reset_editor(editor);
        CharKeyboardEvent event;
        event.key = ch;
        handler(event);
        ASSERT_EQ(editor.lines[0].size(), 1u)
            << "Failed for special: " << ch;
    }
}

TEST(EditorKeyHandlerTest, RejectsInvalidChars) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    EditorKeyHandler handler(editor);

    const std::vector<char> rejected = {
        '!', '@', '$', '%', '^', '&', '*', '(', ')', '=', '+',
        '[', ']', '{', '}', '|', '\\', '"', '\'', '<', '>', '?',
        '`', '~'};
    for(char ch : rejected) {
        reset_editor(editor);
        CharKeyboardEvent event;
        event.key = ch;
        handler(event);
        EXPECT_EQ(editor.lines[0].size(), 0u)
            << "Should have rejected char: " << ch;
    }
}

TEST(NewLineHandlerTest, AddsNewLine) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    NewLineHandler handler(editor);

    const auto before = editor.lines.size();
    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.lines.size(), before + 1);
}

TEST(BackspaceHandlerTest, RemovesCharacter) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    BackspaceHandler handler(editor);

    editor.insert('A');
    ASSERT_EQ(editor.line_length(), 1);
    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.line_length(), 0);
}

TEST(BackspaceHandlerTest, AtStartMergesWithPrevious) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    BackspaceHandler handler(editor);

    editor.insert('A');
    editor.new_line();
    editor.insert('B');
    editor.move_to_start_line();
    ASSERT_EQ(editor.lines.size(), 2u);

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.lines.size(), 1u);
    EXPECT_EQ(editor.lines[0], "AB");
}

TEST(MoveCursorLeftHandlerTest, MovesLeft) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorLeftHandler handler(editor);

    editor.insert('A');
    editor.insert('B');
    ASSERT_EQ(editor.get_cursor_x(), 2);

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_x(), 1);
}

TEST(MoveCursorLeftHandlerTest, WrapsToEndOfPreviousLine) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorLeftHandler handler(editor);

    editor.insert('A');
    editor.new_line();
    editor.move_to_start_line();
    ASSERT_EQ(editor.get_cursor_x(), 0);
    ASSERT_EQ(editor.get_cursor_y(), 1);

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_y(), 0);
    EXPECT_EQ(editor.get_cursor_x(), 1);
}

TEST(MoveCursorLeftHandlerTest, DoesNothingAtOrigin) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorLeftHandler handler(editor);

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_x(), 0);
    EXPECT_EQ(editor.get_cursor_y(), 0);
}

TEST(MoveCursorRightHandlerTest, MovesRight) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorRightHandler handler(editor);

    editor.insert('A');
    editor.insert('B');
    editor.move_to_start_line();

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_x(), 1);
}

TEST(MoveCursorRightHandlerTest, WrapsToStartOfNextLine) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorRightHandler handler(editor);

    editor.insert('A');
    editor.new_line();
    editor.insert('B');
    editor.move_up();
    editor.move_to_end_line();

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_y(), 1);
    EXPECT_EQ(editor.get_cursor_x(), 0);
}

TEST(MoveCursorRightHandlerTest, DoesNothingAtEnd) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorRightHandler handler(editor);

    editor.insert('A');
    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_x(), 1);
    EXPECT_EQ(editor.get_cursor_y(), 0);
}

TEST(MoveCursorUpHandlerTest, MovesUp) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorUpHandler handler(editor);

    editor.insert('A');
    editor.new_line();
    editor.insert('B');
    ASSERT_EQ(editor.get_cursor_y(), 1);

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_y(), 0);
}

TEST(MoveCursorUpHandlerTest, DoesNothingAtTopEdge) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorUpHandler handler(editor);

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_y(), 0);
}

TEST(MoveCursorUpHandlerTest, ClampsXToLineLength) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorUpHandler handler(editor);

    editor.insert('A');
    editor.new_line();
    editor.insert('X');
    editor.insert('Y');
    editor.insert('Z');
    ASSERT_EQ(editor.get_cursor_x(), 3);

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_y(), 0);
    EXPECT_EQ(editor.get_cursor_x(), 1);
}

TEST(MoveCursorDownHandlerTest, MovesDown) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorDownHandler handler(editor);

    editor.insert('A');
    editor.new_line();
    editor.insert('B');
    editor.move_up();

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_y(), 1);
}

TEST(MoveCursorDownHandlerTest, MovesToEndOfLineAtBottom) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorDownHandler handler(editor);

    editor.insert('A');
    editor.insert('B');
    editor.move_to_start_line();

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_y(), 0);
    EXPECT_EQ(editor.get_cursor_x(), 2);
}

TEST(MoveCursorDownHandlerTest, ClampsXToLineLength) {
    CommandMap map;
    SoftwareManager sm(map);
    TextEditor editor(sm);
    MoveCursorDownHandler handler(editor);

    editor.insert('X');
    editor.insert('Y');
    editor.insert('Z');
    editor.new_line();
    editor.insert('A');
    editor.move_up();
    editor.move_to_end_line();
    ASSERT_EQ(editor.get_cursor_x(), 3);

    KeyboardEvent event;
    handler(event);
    EXPECT_EQ(editor.get_cursor_y(), 1);
    EXPECT_EQ(editor.get_cursor_x(), 1);
}