#pragma once

#include <string>
#include <vector>

struct TextEditor {
    std::vector<std::string> lines;
    int cursorX = 0, cursorY = 0;

    TextEditor();
    void moveToPrevNonWhiteSpace();
    void moveToEndLine();
};
