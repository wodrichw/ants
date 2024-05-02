#include "ui/text_editor.hpp"

#include "app/globals.hpp"
#include "ui/event_system.hpp"
#include "spdlog/spdlog.h"

TextEditor::TextEditor()
    : lines(globals::TEXTBOXHEIGHT) {
    SPDLOG_DEBUG("TextEditor constructor - lines size: {}",
                 globals::TEXTBOXHEIGHT);
    for(int i = 0; i < globals::TEXTBOXHEIGHT; ++i) {
        lines[i] = std::string(globals::TEXTBOXWIDTH, ' ');
    }
    SPDLOG_TRACE("TextEditor constructor - lines[0]: {}",
                 lines[0]);
}

void TextEditor::moveToPrevNonWhiteSpace() {
    SPDLOG_TRACE("moveToPrevNonWhiteSpace - cursorX: {}, cursorY: {}", cursorX, cursorY);
    while(cursorX > 0 && lines[cursorY][cursorX - 1] == ' ')
        --cursorX;
    SPDLOG_TRACE("moveToPrevNonWhiteSpace - new cursorX: {}, new cursorY: {}", cursorX, cursorY);
}

void TextEditor::moveToEndLine() {
    SPDLOG_DEBUG("Moving to end of line");
    cursorX = globals::TEXTBOXWIDTH - 1;
    moveToPrevNonWhiteSpace();
    SPDLOG_DEBUG("moveToEndLine - cursorX: {}, cursorY: {}", cursorX, cursorY);
}
