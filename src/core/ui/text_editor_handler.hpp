#pragma once

#include <SDL_keycode.h>

#include <string>
#include <vector>

#include "spdlog/spdlog.h"
#include "app/globals.hpp"
#include "ui/subscriber.hpp"

struct TextEditor {
    std::vector<std::string> lines;
    int cursorX = 0, cursorY = 0;
    bool is_active = false;

    TextEditor();
    void moveToPrevNonWhiteSpace();
    void moveToEndLine();
};


class NewLineHandler: public Subscriber<KeyboardEvent> {
    TextEditor& editor;
public:
    NewLineHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) {
        if (!editor.is_active) return;
        if (editor.cursorY >= (globals::TEXTBOXHEIGHT - 1)) return;
    
        SPDLOG_TRACE("Adding new line at cursorY: {}", editor.cursorY);
        ++editor.cursorY;
        editor.lines.insert(
            editor.lines.begin() + editor.cursorY,
            std::string(globals::TEXTBOXWIDTH, ' '));
        editor.lines.pop_back();
        editor.moveToPrevNonWhiteSpace();
        SPDLOG_TRACE("Added new line at cursorY: {}", editor.cursorY);
    }
};
class BackspaceHandler: public Subscriber<KeyboardEvent> {
    TextEditor& editor;
public:
    BackspaceHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) {
        if (!editor.is_active) return;

        SPDLOG_DEBUG("Backspace key pressed - cursorX: {}, cursorY: {}", editor.cursorX, editor.cursorY);
        if(editor.cursorX > 0) {
            SPDLOG_TRACE("Erasing character at cursorX: {}", editor.cursorX);
            editor.lines[editor.cursorY].erase(editor.cursorX - 1, 1);
            editor.lines[editor.cursorY].push_back(' ');
            if(editor.cursorX > 0) editor.cursorX--;
        } else if(editor.cursorY > 0) {
            SPDLOG_TRACE("Erasing newline at cursorY: {}", editor.cursorY);
            editor.lines.erase(editor.lines.begin() + editor.cursorY);
            editor.lines.push_back(std::string(globals::TEXTBOXWIDTH, ' '));
            --editor.cursorY;
            editor.moveToEndLine();
        }
        SPDLOG_TRACE("Backspace key pressed - cursorX: {}, cursorY: {}", editor.cursorX, editor.cursorY);
    }
};

class MoveCursorLeftHandler: public Subscriber<KeyboardEvent> {
    TextEditor& editor;
public:
    MoveCursorLeftHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) {
        if (!editor.is_active) return;

        SPDLOG_DEBUG("Left key pressed - cursorX: {}, cursorY: {}", editor.cursorX, editor.cursorY);
        if(editor.cursorX > 0) {
            SPDLOG_TRACE("Moving cursor left");
            editor.cursorX--;
        } else if(editor.cursorY > 0) {
            SPDLOG_TRACE("Moving cursor up");
            --editor.cursorY;
            editor.moveToEndLine();
        }
    }
};

class MoveCursorRightHandler: public Subscriber<KeyboardEvent> {
    TextEditor& editor;
public:
    MoveCursorRightHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) {
        if (!editor.is_active) return;

        SPDLOG_DEBUG("Right key pressed - cursorX: {}, cursorY: {}", editor.cursorX, editor.cursorY);
        if(editor.cursorX < (globals::TEXTBOXWIDTH - 1)) {
            SPDLOG_TRACE("Moving cursor right");
            ++editor.cursorX;
        } else if(editor.cursorY < (globals::TEXTBOXHEIGHT - 1)) {
            SPDLOG_TRACE("Moving cursor down");
            ++editor.cursorY;
            editor.cursorX = 0;
        }
        
    }
};

class MoveCursorUpHandler: public Subscriber<KeyboardEvent> {
    TextEditor& editor;
public:
    MoveCursorUpHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) {
        if (!editor.is_active) return;
        if (editor.cursorY <= 0) return;

        SPDLOG_DEBUG("Up key pressed - cursorX: {}, cursorY: {}", editor.cursorX, editor.cursorY);
        --editor.cursorY;
        editor.moveToPrevNonWhiteSpace();
        SPDLOG_TRACE("Up key pressed - new cursorX: {}, new cursorY: {}", editor.cursorX, editor.cursorY);
    }
};

class MoveCursorDownHandler: public Subscriber<KeyboardEvent> {
    TextEditor& editor;
public:
    MoveCursorDownHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) {
        if (!editor.is_active) return;
        if (editor.cursorY >= (globals::TEXTBOXHEIGHT - 1)) return;

        SPDLOG_DEBUG("Down key pressed - cursorX: {}, cursorY: {}", editor.cursorX, editor.cursorY);
        ++editor.cursorY;
        editor.moveToPrevNonWhiteSpace();
        SPDLOG_TRACE("Down key pressed - new cursorX: {}, new cursorY: {}", editor.cursorX, editor.cursorY);
        
    }
};

class TextEditorTriggerHandler: public Subscriber<KeyboardEvent> {
    TextEditor& editor;
public:
    TextEditorTriggerHandler(TextEditor& editor) : editor(editor) {}
    void operator()(KeyboardEvent const&) {
        editor.is_active = !editor.is_active;
    }
};

class EditorKeyHandler: public Subscriber<CharKeyboardEvent> {
    TextEditor& editor;
public:
    EditorKeyHandler(TextEditor& editor) : editor(editor) {}
    void operator()(CharKeyboardEvent const& event) {
        if (!editor.is_active) return;
        if (editor.cursorX >= (globals::TEXTBOXWIDTH - 1)) return;

        if (!(((event.key >= 'a' && event.key <= 'z') ||
            (event.key >= '0' && event.key <= '9') ||
            event.key == ',' || event.key == ' ' ||
            event.key == '#' || event.key == ':'))) return;
    
        SPDLOG_TRACE("Printable key pressed - cursorX: {}, cursorY: {}", editor.cursorX, editor.cursorY);
        editor.lines[editor.cursorY][editor.cursorX] = toupper(event.key);
        editor.cursorX++;
    }
};

