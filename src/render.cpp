#include "render.hpp"

#include <cassert>
#include <numeric>

#include "globals.hpp"
#include "text_editor_handler.hpp"

struct Box {
    ulong x, y, w, h;
    std::vector<std::string> &asciiGrid;
    Box(std::vector<std::string> &asciiGrid, long x, long y, int w, int h)
        : x(x), y(y), w(w), h(h), asciiGrid(asciiGrid) {}

    void populateChar(long x_idx, long y_idx, char ch) {
        asciiGrid[y_idx + y][x + x_idx] = ch;
    }

    void checkInputText(const std::vector<std::string> &text) {
        assert(text.size() == h - 2);
        bool checkStrLengths = std::all_of(
            text.begin(), text.end(),
            [this](const std::string &str) { return str.length() == w - 2; });
        assert(checkStrLengths);
    }

    void populate(const std::vector<std::string> &text) {
        checkInputText(text);

        // render corners
        populateChar(0, 0, '+');
        populateChar(0, h - 1, '+');
        populateChar(w - 1, 0, '+');
        populateChar(w - 1, h - 1, '+');
        for(ulong i = 1; i < h - 1; ++i) {
            populateChar(0, i, '|');
            populateChar(w - 1, i, '|');
        }
        for(ulong i = 1; i < w - 1; ++i) {
            populateChar(i, 0, '-');
            populateChar(i, h - 1, '-');
        }

        for(ulong i = 1; i < h - 1; ++i) {
            for(ulong j = 1; j < w - 1; ++j) {
                populateChar(j, i, text[i - 1][j - 1]);
            }
        }
    }
};

tcodRenderer::tcodRenderer() {
    auto params = TCOD_ContextParams();
    params.columns = globals::COLS, params.rows = globals::ROWS,
    params.window_title = "A N T S";
    context = tcod::Context(params);
    root_console = context.new_console(globals::COLS, globals::ROWS);
}

void tcodRenderer::clearCh(long x, long y) {
    auto &tile = root_console.at(x, y);
    tile.ch = ' ';
}

void tcodRenderer::renderMap(Map &map) {
    TCOD_ColorRGBA darkWall = color::light_black;
    TCOD_ColorRGBA darkGround = color::dark_grey;
    TCOD_ColorRGBA lightWall = color::indian_red;
    TCOD_ColorRGBA lightGround = color::grey;

    for(long x = 0; x < map.width; x++) {
        for(long y = 0; y < map.height; y++) {
            auto &tile = root_console.at(x, y);
            tile.ch = ' ';
            if(map.isInFov(x, y)) {
                tile.bg = map.isWall(x, y) ? lightWall : lightGround;
            } else {
                if(map.isExplored(x, y)) {
                    tile.bg = map.isWall(x, y) ? darkWall : darkGround;
                } else {
                    tile.bg = darkWall;
                }
            }
        }
    }
}

void tcodRenderer::renderAnt(Map &map, Ant &a) {
    PositionData &last_pos = a.last_rendered_pos;
    if(last_pos.requires_update) {
        last_pos.requires_update = false;
        clearCh(last_pos.x, last_pos.y);
    }

    if(map.isInFov(a.x, a.y)) {
        auto &tile = root_console.at(a.x, a.y);
        tile.ch = a.ch;
        tile.fg = a.col;
        last_pos.x = a.x;
        last_pos.y = a.y;
    }
}

void tcodRenderer::renderBuilding(Building &b) {
    for(long xi = b.x; xi < b.x + b.w; ++xi) {
        for(long yi = b.y; yi < b.y + b.h; ++yi) {
            auto &tile = root_console.at(xi, yi);
            tile.bg = b.color;
        }
    }
}

void tcodRenderer::renderTextEditor(TextEditorHandler const &editor,
                                    size_t ant_count) {
    std::vector<std::string> asciiGrid(editor.textBoxHeight + 2);
    for(int i = 0; i < editor.textBoxHeight + 2; ++i) {
        asciiGrid[i] =
            std::string(editor.regBoxWidth + editor.textBoxWidth + 4, ' ');
        asciiGrid[i][editor.regBoxWidth + editor.textBoxWidth + 3] = '\n';
    }

    Box mainBox(asciiGrid, 0, 0, editor.textBoxWidth + 2,
                editor.textBoxHeight + 2);
    Box accBox(asciiGrid, editor.textBoxWidth + 1, 0, editor.regBoxWidth + 2,
               editor.regBoxHeight + 2);
    Box bacBox(asciiGrid, editor.textBoxWidth + 1, editor.regBoxHeight + 1,
               editor.regBoxWidth + 2, editor.regBoxHeight + 2);
    Box antBox(asciiGrid, editor.textBoxWidth + 1,
               (editor.regBoxHeight * 2) + 2, editor.regBoxWidth + 2,
               editor.regBoxHeight + 2);

    mainBox.populate(editor.textEditorLines);
    accBox.populate({"ACC:0   "});
    bacBox.populate({"BAC:1   "});

    int numAntsSpaces = 4 - std::to_string(ant_count).length();
    std::ostringstream numAntsStream;
    numAntsStream << "ANT:" << ant_count << std::string(numAntsSpaces, ' ');
    antBox.populate({numAntsStream.str()});

    std::string result =
        std::accumulate(asciiGrid.begin(), asciiGrid.end(), std::string(""));

    tcod::print_rect(root_console,
                     {0, 0, editor.textBoxWidth + editor.regBoxWidth + 4,
                      editor.textBoxHeight + editor.regBoxHeight + 3},
                     result, color::white, color::black, TCOD_LEFT,
                     TCOD_BKGND_SET);

    tcod::print_rect(
        root_console, {editor.cursorX + 1, editor.cursorY + 1, 1, 1}, " ",
        color::white, color::light_green, TCOD_LEFT, TCOD_BKGND_SET);
}

// TODO: display potential key presses that could be helpful.
// For instance, when standing in a nursery, display keys to produce new
// workers. This could be replaced with something else in the future.
void tcodRenderer::renderHelpBoxes() {}

void tcodRenderer::present() { context.present(root_console); }

void tcodRenderer::pixel_to_tile_coordinates(int pixel_x, int pixel_y,
                                             long &tile_x, long &tile_y) {
    std::array<int, 2> tile =
        context.pixel_to_tile_coordinates(std::array<int, 2>{pixel_x, pixel_y});
    tile_x = tile[0];
    tile_y = tile[1];
}
