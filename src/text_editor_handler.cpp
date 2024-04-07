#include "text_editor_handler.hpp"

#include "globals.hpp"
#include "spdlog/spdlog.h"

TextEditorHandler::TextEditorHandler(Map* map)
    : textEditorLines(globals::TEXTBOXHEIGHT), map(map) {
    SPDLOG_DEBUG("TextEditorHandler constructor - textEditorLines size: {}",
                 globals::TEXTBOXHEIGHT);
    for(int i = 0; i < globals::TEXTBOXHEIGHT; ++i) {
        textEditorLines[i] = std::string(globals::TEXTBOXWIDTH, ' ');
    }
    SPDLOG_TRACE("TextEditorHandler constructor - textEditorLines[0]: {}",
                 textEditorLines[0]);
}

void TextEditorHandler::moveToPrevNonWhiteSpace() {
    SPDLOG_TRACE("moveToPrevNonWhiteSpace - cursorX: {}, cursorY: {}", cursorX, cursorY);
    while(cursorX > 0 && textEditorLines[cursorY][cursorX - 1] == ' ')
        --cursorX;
    SPDLOG_TRACE("moveToPrevNonWhiteSpace - new cursorX: {}, new cursorY: {}", cursorX, cursorY);
}

void TextEditorHandler::moveToEndLine() {
    SPDLOG_DEBUG("Moving to end of line");
    cursorX = globals::TEXTBOXWIDTH - 1;
    moveToPrevNonWhiteSpace();
    SPDLOG_DEBUG("moveToEndLine - cursorX: {}, cursorY: {}", cursorX, cursorY);
}

void TextEditorHandler::handleTextEditorAction(SDL_Keycode key_sym) {
    SPDLOG_DEBUG("handleTextEditorAction - key_sym: {}", key_sym);
    bool key_is_printable = cursorX < (globals::TEXTBOXWIDTH - 1) &&
                            ((key_sym >= SDLK_a && key_sym <= SDLK_z) ||
                             (key_sym >= SDLK_0 && key_sym <= SDLK_9) ||
                             key_sym == SDLK_COMMA || key_sym == SDLK_SPACE ||
                             key_sym == SDLK_HASH || key_sym == SDLK_COLON);

    if(key_sym == SDLK_RETURN && cursorY < (globals::TEXTBOXHEIGHT - 1)) {
        SPDLOG_TRACE("Adding new line at cursorY: {}", cursorY);
        ++cursorY;
        textEditorLines.insert(textEditorLines.begin() + cursorY,
                               std::string(globals::TEXTBOXWIDTH, ' '));
        textEditorLines.pop_back();
        moveToPrevNonWhiteSpace();
        SPDLOG_TRACE("Added new line at cursorY: {}", cursorY);
    } else if(key_sym == SDLK_BACKSPACE) {
        SPDLOG_DEBUG("Backspace key pressed - cursorX: {}, cursorY: {}", cursorX, cursorY);
        if(cursorX > 0) {
            SPDLOG_TRACE("Erasing character at cursorX: {}", cursorX);
            textEditorLines[cursorY].erase(cursorX - 1, 1);
            textEditorLines[cursorY].push_back(' ');
            if(cursorX > 0) cursorX--;
        } else if(cursorY > 0) {
            SPDLOG_TRACE("Erasing newline at cursorY: {}", cursorY);
            textEditorLines.erase(textEditorLines.begin() + cursorY);
            textEditorLines.push_back(std::string(globals::TEXTBOXWIDTH, ' '));
            --cursorY;
            moveToEndLine();
        }
        SPDLOG_TRACE("Backspace key pressed - cursorX: {}, cursorY: {}", cursorX, cursorY);
    } else if(key_is_printable) {
        SPDLOG_TRACE("Printable key pressed - cursorX: {}, cursorY: {}", cursorX, cursorY);
        textEditorLines[cursorY][cursorX] = toupper(key_sym);
        cursorX++;
    } else if(key_sym == SDLK_LEFT) {
        SPDLOG_DEBUG("Left key pressed - cursorX: {}, cursorY: {}", cursorX, cursorY);
        if(cursorX > 0) {
            SPDLOG_TRACE("Moving cursor left");
            cursorX--;
        } else if(cursorY > 0) {
            SPDLOG_TRACE("Moving cursor up");
            --cursorY;
            moveToEndLine();
        }

    } else if(key_sym == SDLK_RIGHT) {
        SPDLOG_DEBUG("Right key pressed - cursorX: {}, cursorY: {}", cursorX, cursorY);
        if(cursorX < (globals::TEXTBOXWIDTH - 1)) {
            SPDLOG_TRACE("Moving cursor right");
            ++cursorX;
        } else if(cursorY < (globals::TEXTBOXHEIGHT - 1)) {
            SPDLOG_TRACE("Moving cursor down");
            ++cursorY;
            cursorX = 0;
        }
    } else if(key_sym == SDLK_UP && cursorY > 0) {
        SPDLOG_DEBUG("Up key pressed - cursorX: {}, cursorY: {}", cursorX, cursorY);
        --cursorY;
        moveToPrevNonWhiteSpace();
        SPDLOG_TRACE("Up key pressed - new cursorX: {}, new cursorY: {}", cursorX, cursorY);
    } else if(key_sym == SDLK_DOWN && cursorY < (globals::TEXTBOXHEIGHT - 1)) {
        SPDLOG_DEBUG("Down key pressed - cursorX: {}, cursorY: {}", cursorX, cursorY);
        ++cursorY;
        moveToPrevNonWhiteSpace();
        SPDLOG_TRACE("Down key pressed - new cursorX: {}, new cursorY: {}", cursorX, cursorY);
    }
}