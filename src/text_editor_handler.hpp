#pragma once

#include <SDL_keycode.h>

#include <string>
#include <vector>

#include "map.hpp"

class TextEditorHandler {
   public:
    TextEditorHandler(Map* map);
    std::vector<std::string> textEditorLines;
    int cursorX = 0, cursorY = 0;
    void handleTextEditorAction(SDL_Keycode key_sym);

   private:
    void moveToPrevNonWhiteSpace();
    void moveToEndLine();
    Map* map;
};
