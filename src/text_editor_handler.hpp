#pragma once

#include <vector>
#include <string>
#include <SDL_keycode.h>

#include "map.hpp"

class TextEditorHandler {
public :
    TextEditorHandler(Map* map);
    std::vector<std::string> textEditorLines;
    int cursorX = 0, cursorY = 0;
    void printTextEditor();
    void printHelpBoxes();
    void handleTextEditorAction(SDL_Keycode key_sym);
private:
    void moveToPrevNonWhiteSpace();
    void moveToEndLine();
    static const int textBoxHeight = 17;
    static const int textBoxWidth = 23;
    static const int regBoxWidth = 6;
    static const int regBoxHeight = 1;
    Map* map;
};
