#include "text_editor_handler.hpp"

#include "globals.hpp"

TextEditorHandler::TextEditorHandler(Map* map)
    : textEditorLines(globals::TEXTBOXHEIGHT), map(map) {
    for(int i = 0; i < globals::TEXTBOXHEIGHT; ++i) {
        textEditorLines[i] = std::string(globals::TEXTBOXWIDTH, ' ');
    }
}

void TextEditorHandler::moveToPrevNonWhiteSpace() {
    while(cursorX > 0 && textEditorLines[cursorY][cursorX - 1] == ' ')
        --cursorX;
}

void TextEditorHandler::moveToEndLine() {
    cursorX = globals::TEXTBOXWIDTH - 1;
    moveToPrevNonWhiteSpace();
}

void TextEditorHandler::handleTextEditorAction(SDL_Keycode key_sym) {
    bool key_is_printable = cursorX < (globals::TEXTBOXWIDTH - 1) &&
                            ((key_sym >= SDLK_a && key_sym <= SDLK_z) ||
                             (key_sym >= SDLK_0 && key_sym <= SDLK_9) ||
                             key_sym == SDLK_COMMA || key_sym == SDLK_SPACE ||
                             key_sym == SDLK_HASH || key_sym == SDLK_COLON);

    if(key_sym == SDLK_RETURN && cursorY < (globals::TEXTBOXHEIGHT - 1)) {
        ++cursorY;
        textEditorLines.insert(textEditorLines.begin() + cursorY,
                               std::string(globals::TEXTBOXWIDTH, ' '));
        textEditorLines.pop_back();
        moveToPrevNonWhiteSpace();
    } else if(key_sym == SDLK_BACKSPACE) {
        if(cursorX > 0) {
            textEditorLines[cursorY].erase(cursorX - 1, 1);
            textEditorLines[cursorY].push_back(' ');
            if(cursorX > 0) cursorX--;
        } else if(cursorY > 0) {
            textEditorLines.erase(textEditorLines.begin() + cursorY);
            textEditorLines.push_back(std::string(globals::TEXTBOXWIDTH, ' '));
            --cursorY;
            moveToEndLine();
        }
    } else if(key_is_printable) {
        textEditorLines[cursorY][cursorX] = toupper(key_sym);
        cursorX++;
    } else if(key_sym == SDLK_LEFT) {
        if(cursorX > 0) {
            cursorX--;
        } else if(cursorY > 0) {
            --cursorY;
            moveToEndLine();
        }

    } else if(key_sym == SDLK_RIGHT) {
        if(cursorX < (globals::TEXTBOXWIDTH - 1)) {
            ++cursorX;
        } else if(cursorY < (globals::TEXTBOXHEIGHT - 1)) {
            ++cursorY;
            cursorX = 0;
        }
    } else if(key_sym == SDLK_UP && cursorY > 0) {
        --cursorY;
        moveToPrevNonWhiteSpace();
    } else if(key_sym == SDLK_DOWN && cursorY < (globals::TEXTBOXHEIGHT - 1)) {
        ++cursorY;
        moveToPrevNonWhiteSpace();
    }
}