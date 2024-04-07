#include "render.hpp"

#include <cassert>
#include <numeric>
#include <sstream>

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

void tcodRenderer::renderMap(LayoutBox const &box, Map &map) {
    TCOD_ColorRGBA darkWall = color::light_black;
    TCOD_ColorRGBA darkGround = color::dark_grey;
    TCOD_ColorRGBA lightWall = color::indian_red;
    TCOD_ColorRGBA lightGround = color::grey;

    for(long x = 0; x < map.width; x++) {
        for(long y = 0; y < map.height; y++) {
            auto &tile = clearCh(box, x, y);
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

void tcodRenderer::renderAnt(LayoutBox const &box, Map &map, Ant &a) {
    PositionData &last_pos = a.last_rendered_pos;
    if(last_pos.requires_update) {
        last_pos.requires_update = false;
        clearCh(box, last_pos.x, last_pos.y);
    }

    if(map.isInFov(a.x, a.y)) {
        auto &tile = get_tile(box, a.x, a.y);
        tile.ch = a.ch;
        tile.fg = a.col;
        last_pos.x = a.x;
        last_pos.y = a.y;
    }
}

void tcodRenderer::renderBuilding(LayoutBox const &box, Building &b) {
    for(long xi = b.x; xi < b.x + b.w; ++xi) {
        for(long yi = b.y; yi < b.y + b.h; ++yi) {
            auto &tile = get_tile(box, xi, yi);
            tile.bg = b.color;
        }
    }
}

void tcodRenderer::renderTextEditor(LayoutBox const &box,
                                    TextEditorHandler const &editor,
                                    size_t ant_count) {
    std::vector<std::string> asciiGrid(globals::TEXTBOXHEIGHT + 2);
    for(int i = 0; i < globals::TEXTBOXHEIGHT + 2; ++i) {
        asciiGrid[i] =
            std::string(globals::REGBOXWIDTH + globals::TEXTBOXWIDTH + 4, ' ');
        asciiGrid[i][globals::REGBOXWIDTH + globals::TEXTBOXWIDTH + 3] = '\n';
    }

    Box mainBox(asciiGrid, 0, 0, globals::TEXTBOXWIDTH + 2,
                globals::TEXTBOXHEIGHT + 2);
    Box accBox(asciiGrid, globals::TEXTBOXWIDTH + 1, 0,
               globals::REGBOXWIDTH + 2, globals::REGBOXHEIGHT + 2);
    Box bacBox(asciiGrid, globals::TEXTBOXWIDTH + 1, globals::REGBOXHEIGHT + 1,
               globals::REGBOXWIDTH + 2, globals::REGBOXHEIGHT + 2);
    Box antBox(asciiGrid, globals::TEXTBOXWIDTH + 1,
               (globals::REGBOXHEIGHT * 2) + 2, globals::REGBOXWIDTH + 2,
               globals::REGBOXHEIGHT + 2);

    mainBox.populate(editor.textEditorLines);
    accBox.populate({"ACC:0   "});
    bacBox.populate({"BAC:1   "});

    int numAntsSpaces = 4 - std::to_string(ant_count).length();
    std::ostringstream numAntsStream;
    numAntsStream << "ANT:" << ant_count << std::string(numAntsSpaces, ' ');
    antBox.populate({numAntsStream.str()});

    std::string result =
        std::accumulate(asciiGrid.begin(), asciiGrid.end(), std::string(""));

    tcod::print_rect(
        root_console,
        get_rect(box, 0, 0, globals::TEXTBOXWIDTH + globals::REGBOXWIDTH + 4,
                 globals::TEXTBOXHEIGHT + globals::REGBOXHEIGHT + 3),
        result, color::white, color::black, TCOD_LEFT, TCOD_BKGND_SET);

    tcod::print_rect(
        root_console,
        get_rect(box, editor.cursorX + 1, editor.cursorY + 1, 1, 1), " ",
        color::white, color::light_green, TCOD_LEFT, TCOD_BKGND_SET);
}

// TODO: display potential key presses that could be helpful.
// For instance, when standing in a nursery, display keys to produce new
// workers. This could be replaced with something else in the future.
void tcodRenderer::renderHelpBoxes(LayoutBox const &box) {}

void tcodRenderer::present() { context.present(root_console); }

void tcodRenderer::pixel_to_tile_coordinates(int pixel_x, int pixel_y,
                                             long &tile_x, long &tile_y) {
    std::array<int, 2> tile =
        context.pixel_to_tile_coordinates(std::array<int, 2>{pixel_x, pixel_y});
    tile_x = tile[0];
    tile_y = tile[1];
}

TCOD_ConsoleTile &tcodRenderer::clearCh(LayoutBox const &box, long x, long y) {
    auto &tile = get_tile(box, x, y);
    tile.ch = ' ';
    return tile;
}

TCOD_ConsoleTile &tcodRenderer::get_tile(LayoutBox const &box, long x, long y) {
    long abs_x = 0, abs_y = 0;
    box.get_abs_pos(x, y, abs_x, abs_y);
    return root_console.at(abs_x, abs_y);
}

const std::array<int, 4> tcodRenderer::get_rect(LayoutBox const &box, long x,
                                                long y, int w, int h) {
    long abs_x = 0, abs_y = 0;
    box.get_abs_pos(x, y, abs_x, abs_y);
    return {(int)abs_x, (int)abs_y, w, h};
}

LayoutBox::LayoutBox() : xp(0), yp(0), wp(0), hp(0), x(0), y(0), w(0), h(0) {}
LayoutBox::LayoutBox(long w, long h)
    : xp(0), yp(0), wp(w), hp(h), x(0), y(0), w(w), h(h) {}
LayoutBox::LayoutBox(long x, long y, long w, long h)
    : xp(x), yp(y), wp(w), hp(h), x(x), y(y), w(w), h(h) {}

LayoutBox::~LayoutBox() {
    if(children.first) delete children.first;
    if(children.second) delete children.second;
}
void LayoutBox::get_abs_pos(long x0, long y0, long &x1, long &y1) const {
    x1 = x + x0;
    y1 = y + y0;
}

long LayoutBox::get_width() const { return this->w; }
long LayoutBox::get_height() const { return this->h; }

std::pair<LayoutBox *, LayoutBox *> &LayoutBox::split(ulong percentage,
                                                      Orientation orientation) {
    if(orientation == Orientation::HORIZONTAL) {
        ulong first_h = h * percentage / 100;
        ulong second_h = h - first_h;
        children.first = new LayoutBox(x, y, w, first_h);
        children.second = new LayoutBox(x, y + first_h, w, second_h);

    } else {
        ulong first_w = w * percentage / 100;
        ulong second_w = w - first_w;
        children.first = new LayoutBox(x, y, first_w, h);
        children.second = new LayoutBox(x + first_w, y, second_w, h);
    }
    return children;
}

BoxManager::BoxManager(ulong w, ulong h) : main(w, h), text_editor_root(w, h) {
    ulong map_split = 80;
    std::tie(map_box, sidebar_box) =
        main.split(map_split, LayoutBox::Orientation::VERTICAL);

    LayoutBox *editor_right_menu = nullptr, *editor_empty = nullptr;
    std::tie(text_editor_content_box, editor_right_menu) =
        text_editor_root.split(map_split, LayoutBox::Orientation::VERTICAL);
    text_editor_content_box->center(
        globals::TEXTBOXWIDTH + globals::REGBOXWIDTH,
        globals::TEXTBOXHEIGHT + globals::REGBOXHEIGHT);
    std::tie(text_editor_registers_box, editor_empty) =
        editor_right_menu->split(30, LayoutBox::Orientation::HORIZONTAL);
}

void LayoutBox::center(ulong new_width, ulong new_height) {
    x = xp + (wp - new_width) / 2;
    y = yp + (hp - new_height) / 2;
    w = new_width;
    h = new_height;
}
