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
    void handleTextEditorAction(SDL_Keycode key_sym);
    
    static const int textBoxHeight = 17;
    static const int textBoxWidth = 23;
    static const int regBoxWidth = 6;
    static const int regBoxHeight = 1;
private:
    void moveToPrevNonWhiteSpace();
    void moveToEndLine();
    Map* map;
};
