#include <algorithm>
#include <cassert>
#include <numeric>

#include "text_editor_handler.hpp"
#include "colors.hpp"

struct Box {
    ulong x, y, w, h;
    std::vector<std::string> &asciiGrid;
    Box(std::vector<std::string> &asciiGrid, int x, int y, int w, int h)
        : x(x), y(y), w(w), h(h), asciiGrid(asciiGrid) {}

    void populateChar(int x_idx, int y_idx, char ch)
    {
        asciiGrid[y_idx + y][x + x_idx] = ch;
    }

    void checkInputText(const std::vector<std::string> &text)
    {
        assert(text.size() == h - 2);
        bool checkStrLengths = std::all_of(text.begin(), text.end(), [this](const std::string &str)
        {
            return str.length() == w - 2;
        });
        assert(checkStrLengths);
    }

    void populate(const std::vector<std::string> &text)
    {
        checkInputText(text);

        // render corners
        populateChar(0, 0, '+');
        populateChar(0, h - 1, '+');
        populateChar(w - 1, 0, '+');
        populateChar(w - 1, h - 1, '+');
        for (ulong i = 1; i < h - 1; ++i) {
            populateChar(0, i, '|');
            populateChar(w - 1, i, '|');
        }
        for (ulong i = 1; i < w - 1; ++i) {
            populateChar(i, 0, '-');
            populateChar(i, h - 1, '-');
        }

        for (ulong i = 1; i < h - 1; ++i) {
            for (ulong j = 1; j < w - 1; ++j) {
                populateChar(j, i, text[i - 1][j - 1]);
            }
        }
    }
};

TextEditorHandler::TextEditorHandler(Map* map): textEditorLines(textBoxHeight), map(map) {
    for (int i = 0; i < textBoxHeight; ++i) {
        textEditorLines[i] = std::string(textBoxWidth, ' ');
    }
}

void TextEditorHandler::printTextEditor()
{
    std::vector<std::string> asciiGrid(textBoxHeight + 2);
    for (int i = 0; i < textBoxHeight + 2; ++i) {
        asciiGrid[i] = std::string(regBoxWidth + textBoxWidth + 4, ' ');
        asciiGrid[i][regBoxWidth + textBoxWidth + 3] = '\n';
    }

    Box mainBox(asciiGrid, 0, 0, textBoxWidth + 2, textBoxHeight + 2);
    Box accBox(asciiGrid, textBoxWidth + 1, 0, regBoxWidth + 2, regBoxHeight + 2);
    Box bacBox(asciiGrid, textBoxWidth + 1, regBoxHeight + 1, regBoxWidth + 2,
            regBoxHeight + 2);

    mainBox.populate(textEditorLines);
    accBox.populate({"ACC:0 "});
    bacBox.populate({"BAC:1 "});

    std::string result =
        std::accumulate(asciiGrid.begin(), asciiGrid.end(), std::string(""));

    tcod::print_rect(
            map->root_console,
            {0, 0, textBoxWidth + regBoxWidth + 4, textBoxHeight + regBoxHeight + 3},
            result, color::white, color::black, TCOD_LEFT, TCOD_BKGND_SET);

    tcod::print_rect(map->root_console, {cursorX + 1, cursorY + 1, 1, 1}, " ",
            color::white, color::light_green, TCOD_LEFT, TCOD_BKGND_SET);
}

// TODO: display potential key presses that could be helpful.
// For instance, when standing in a nursery, display keys to produce new
// workers. This could be replaced with something else in the future.
void TextEditorHandler::printHelpBoxes() {}

void TextEditorHandler::moveToPrevNonWhiteSpace() {
    while (cursorX > 0 && textEditorLines[cursorY][cursorX - 1] == ' ')
        --cursorX;
}

void TextEditorHandler::moveToEndLine() {
    cursorX = textBoxWidth - 1;
    moveToPrevNonWhiteSpace();
}

void TextEditorHandler::handleTextEditorAction(SDL_Keycode key_sym)
{
    bool key_is_printable = cursorX < (textBoxWidth - 1) && (
        (key_sym >= SDLK_a && key_sym <= SDLK_z) ||
        (key_sym >= SDLK_0 && key_sym <= SDLK_9) ||
        key_sym == SDLK_COMMA || key_sym == SDLK_SPACE ||
        key_sym == SDLK_HASH || key_sym == SDLK_COLON
    );

    if (key_sym == SDLK_RETURN && cursorY < (textBoxHeight - 1)) {
        ++cursorY;
        textEditorLines.insert(textEditorLines.begin() + cursorY,
                std::string(textBoxWidth, ' '));
        textEditorLines.pop_back();
        moveToPrevNonWhiteSpace();
    } else if (key_sym == SDLK_BACKSPACE) {
        if (cursorX > 0) {
            textEditorLines[cursorY].erase(cursorX - 1, 1);
            textEditorLines[cursorY].push_back(' ');
            if (cursorX > 0)
                cursorX--;
        } else if (cursorY > 0) {
            textEditorLines.erase(textEditorLines.begin() + cursorY);
            textEditorLines.push_back(std::string(textBoxWidth, ' '));
            --cursorY;
            moveToEndLine();
        }
    } else if( key_is_printable ) {
        textEditorLines[cursorY][cursorX] = toupper(key_sym);
        cursorX++;
    } else if (key_sym == SDLK_LEFT) {
        if (cursorX > 0) {
            cursorX--;
        } else if (cursorY > 0) {
            --cursorY;
            moveToEndLine();
        }

    } else if (key_sym == SDLK_RIGHT) {
        if (cursorX < (textBoxWidth - 1)) {
            ++cursorX;
        } else if (cursorY < (textBoxHeight - 1)) {
            ++cursorY;
            cursorX = 0;
        }
    } else if (key_sym == SDLK_UP && cursorY > 0) {
        --cursorY;
        moveToPrevNonWhiteSpace();
    } else if (key_sym == SDLK_DOWN && cursorY < (textBoxHeight - 1)) {
        ++cursorY;
        moveToPrevNonWhiteSpace();
    }
}