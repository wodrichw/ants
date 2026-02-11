#pragma once

#include <SDL_keycode.h>

#include <cctype>

#include "app/game_state.hpp"
#include "ui/text_editor.hpp"

class NewLineHandler : public Subscriber<KeyboardEvent> {
    TextEditor& editor;

   public:
    NewLineHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) { editor.new_line(); }
};
class BackspaceHandler : public Subscriber<KeyboardEvent> {
    TextEditor& editor;

   public:
    BackspaceHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) { editor.backspace(); }
};

class MoveCursorLeftHandler : public Subscriber<KeyboardEvent> {
    TextEditor& editor;

   public:
    MoveCursorLeftHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) {
        if(!editor.on_left_edge()) {
            editor.move_left();
        } else if(!editor.on_top_edge()) {
            editor.move_up();
            editor.move_to_end_line();
        }
    }
};

class MoveCursorRightHandler : public Subscriber<KeyboardEvent> {
    TextEditor& editor;

   public:
    MoveCursorRightHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) {
        if(!editor.on_right_edge()) {
            editor.move_right();
        } else if(!editor.on_bottom_edge()) {
            editor.move_down();
            editor.move_to_start_line();
        }
    }
};

class MoveCursorUpHandler : public Subscriber<KeyboardEvent> {
    TextEditor& editor;

   public:
    MoveCursorUpHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) {
        if(editor.on_top_edge()) return;

        editor.move_up();
        editor.go_to_text_x();
    }
};

class MoveCursorDownHandler : public Subscriber<KeyboardEvent> {
    TextEditor& editor;

   public:
    MoveCursorDownHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) {
        if(editor.on_bottom_edge()) {
            editor.move_to_end_line();
            return;
        }

        editor.move_down();
        editor.go_to_text_x();
    }
};

class TextEditorTriggerHandler : public Subscriber<KeyboardEvent> {
    GameState& state;

   public:
    TextEditorTriggerHandler(GameState& state) : state(state) {}
    void operator()(KeyboardEvent const&) { state.toggle_editor(); }
};

class EditorKeyHandler : public Subscriber<CharKeyboardEvent> {
    TextEditor& editor;

   public:
    EditorKeyHandler(TextEditor& editor) : editor(editor) {}
    void operator()(CharKeyboardEvent const& event) {
        if(!(((event.key >= 'a' && event.key <= 'z') ||
              (event.key >= '0' && event.key <= '9') || event.key == ',' ||
              event.key == ' ' || event.key == '#' || event.key == ':' ||
              event.key == '/') ||
             event.key == '-'))
            return;

        editor.insert(static_cast<char>(
            std::toupper(static_cast<unsigned char>(event.key))));
    }
};
