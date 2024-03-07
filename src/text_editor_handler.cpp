#include "text_editor_handler.hpp"

TextEditorHandler::TextEditorHandler(Map* map)
    : textEditorLines(textBoxHeight), map(map) {
    for(int i = 0; i < textBoxHeight; ++i) {
        textEditorLines[i] = std::string(textBoxWidth, ' ');
    }
}

void TextEditorHandler::moveToPrevNonWhiteSpace() {
    while(cursorX > 0 && textEditorLines[cursorY][cursorX - 1] == ' ')
        --cursorX;
}

void TextEditorHandler::moveToEndLine() {
    cursorX = textBoxWidth - 1;
    moveToPrevNonWhiteSpace();
}

void TextEditorHandler::handleTextEditorAction(SDL_Keycode key_sym) {
    bool key_is_printable = cursorX < (textBoxWidth - 1) &&
                            ((key_sym >= SDLK_a && key_sym <= SDLK_z) ||
                             (key_sym >= SDLK_0 && key_sym <= SDLK_9) ||
                             key_sym == SDLK_COMMA || key_sym == SDLK_SPACE ||
                             key_sym == SDLK_HASH || key_sym == SDLK_COLON);

    if(key_sym == SDLK_RETURN && cursorY < (textBoxHeight - 1)) {
        ++cursorY;
        textEditorLines.insert(textEditorLines.begin() + cursorY,
                               std::string(textBoxWidth, ' '));
        textEditorLines.pop_back();
        moveToPrevNonWhiteSpace();
    } else if(key_sym == SDLK_BACKSPACE) {
        if(cursorX > 0) {
            textEditorLines[cursorY].erase(cursorX - 1, 1);
            textEditorLines[cursorY].push_back(' ');
            if(cursorX > 0) cursorX--;
        } else if(cursorY > 0) {
            textEditorLines.erase(textEditorLines.begin() + cursorY);
            textEditorLines.push_back(std::string(textBoxWidth, ' '));
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
        if(cursorX < (textBoxWidth - 1)) {
            ++cursorX;
        } else if(cursorY < (textBoxHeight - 1)) {
            ++cursorY;
            cursorX = 0;
        }
    } else if(key_sym == SDLK_UP && cursorY > 0) {
        --cursorY;
        moveToPrevNonWhiteSpace();
    } else if(key_sym == SDLK_DOWN && cursorY < (textBoxHeight - 1)) {
        ++cursorY;
        moveToPrevNonWhiteSpace();
    }
}