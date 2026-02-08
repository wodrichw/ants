#include "ui/render.hpp"
#include "app/clock_speed.hpp"

#include <cassert>
#include <numeric>
#include <sstream>

#include "app/globals.hpp"
#include "spdlog/spdlog.h"
#include "ui/colors.hpp"
#include "ui/debug_graphics.hpp"
#include "utils/types.hpp"

struct Box {
    long x, y, w, h;
    std::vector<std::string> &asciiGrid;
    Box(std::vector<std::string> &asciiGrid, long x, long y, int w, int h)
        : x(x), y(y), w(w), h(h), asciiGrid(asciiGrid) {
        // SPDLOG_TRACE("Box created at ({}, {}) with dimensions {}x{}", x, y,
        // w, h);
    }

    void populate_char(long x_idx, long y_idx, char ch) {
        asciiGrid[y_idx + y][x + x_idx] = ch;
        // SPDLOG_TRACE("Populated char {} at ({}, {})", ch, x_idx, y_idx);
    }

    void populate(const std::vector<std::string> &text, ushort offset_x = 0,
                  ushort offset_y = 0) {
        // SPDLOG_TRACE("Populating box with {} lines", text.size());

        // render corners
        // SPDLOG_DEBUG("Rendering corners");
        populate_char(0, 0, '+');
        populate_char(0, h - 1, '+');
        populate_char(w - 1, 0, '+');
        populate_char(w - 1, h - 1, '+');
        for(long i = 1; i < h - 1; ++i) {
            populate_char(0, i, '|');
            populate_char(w - 1, i, '|');
        }
        for(long i = 1; i < w - 1; ++i) {
            populate_char(i, 0, '-');
            populate_char(i, h - 1, '-');
        }

        for(long i = 1; i < h - 1; ++i) {
            auto text_line_idx = static_cast<size_t>(offset_y + i - 1);
            bool is_line_filled = text_line_idx < text.size();
            for(long j = 1; j < w - 1; ++j) {
                auto char_idx = static_cast<size_t>(offset_x + j - 1);
                populate_char(
                    j, i,
                    is_line_filled && char_idx < text[text_line_idx].size()
                        ? text[text_line_idx][char_idx]
                        : ' ');
            }
        }
    }
};

tcodRenderer::tcodRenderer(bool is_debug_graphics)
    : is_debug_graphics(is_debug_graphics) {
    SPDLOG_DEBUG("Creating tcod renderer");
    auto params = TCOD_ContextParams();
    params.columns = globals::COLS, params.rows = globals::ROWS,
    SPDLOG_DEBUG("Set window size to {}x{}", params.columns, params.rows);

    params.window_title = "A N T S";
    SPDLOG_TRACE("Set window title to {}", params.window_title);

    context = tcod::Context(params);
    root_console = context.new_console(globals::COLS, globals::ROWS);
    SPDLOG_TRACE("Created root console - completed creating tcod renderer");

    if(is_debug_graphics)
        use_debug_tile_rendering();
    else
        use_default_tile_rendering();
}

void tcodRenderer::render_map(LayoutBox const &box, Map &map,
                              MapWindow const &window) {
    std::unique_ptr<MapTileRenderer> tile_renderer =
        generate_tile_renderer(map);

    // SPDLOG_TRACE("Rendering map with border ({}, {}) - {}x{}",
    for(long local_x = 0; local_x < window.border.w; ++local_x) {
        for(long local_y = 0; local_y < window.border.h; ++local_y) {
            long x = local_x + window.border.x1;
            long y = local_y + window.border.y1;

            // SPDLOG_TRACE("Rendering tile at ({}, {}) - local ({}, {})", x, y,
            // local_x, local_y);
            auto &tile = clear_tile(box, local_x, local_y);
            (*tile_renderer)(tile, x, y);
        }
    }
    // render the debug info on the screen
    DEBUG_CHUNKS(
        box, map, window,
        [this](LayoutBox const &box, long x, long y) -> TCOD_ConsoleTile & {
            return this->get_tile(box, x, y);
        },
        is_debug_graphics);
}

DebugMapTileRenderer::DebugMapTileRenderer(Map &map) : map(map) {}

void DebugMapTileRenderer::operator()(TCOD_ConsoleTile &tile, long x, long y) {
    TCOD_ColorRGBA darkWall = color::light_black;
    TCOD_ColorRGBA lightWall = color::indian_red;
    TCOD_ColorRGBA lightGround = color::grey;

    if(map.in_fov(x, y)) {
        tile.bg = map.is_wall(x, y) ? lightWall : lightGround;
    } else {
        tile.bg = map.is_wall(x, y) ? darkWall : lightGround;
    }
}

TcodMapTileRenderer::TcodMapTileRenderer(Map &map) : map(map) {}

void TcodMapTileRenderer::operator()(TCOD_ConsoleTile &tile, long x, long y) {
    // SPDLOG_TRACE("Rendering map");
    TCOD_ColorRGBA darkWall = color::light_black;
    TCOD_ColorRGBA darkGround = color::dark_grey;
    TCOD_ColorRGBA lightWall = color::indian_red;
    TCOD_ColorRGBA lightGround = color::grey;

    // handle debug case
    if(is_debug_graphics) {
        return;
    }
    if(map.in_fov(x, y)) {
        tile.bg = map.is_wall(x, y) ? lightWall : lightGround;
    } else {
        tile.bg =
            map.is_wall(x, y) || !map.is_explored(x, y) ? darkWall : darkGround;
    }
}

ScentMapTileRenderer::ScentMapTileRenderer(Map &map, ulong scent_idx)
    : map(map), scent_idx(scent_idx) {
    SPDLOG_TRACE("Created scent map tile renderer with scent_idx: {}",
                 scent_idx);
}

void ScentMapTileRenderer::operator()(TCOD_ConsoleTile &tile, long x, long y) {
    // SPDLOG_TRACE("Rendering map");
    TCOD_ColorRGBA darkWall = color::light_black;
    TCOD_ColorRGBA darkGround = color::dark_grey;
    TCOD_ColorRGBA lightWall = color::indian_red;

    ulong scents = map.get_tile_scents_by_coord(x, y);
    unsigned char scent = static_cast<signed char>(scents >> scent_idx);
    TCOD_ColorRGBA scent_color{scent, scent, scent, 255};

    if(map.in_fov(x, y)) {
        tile.bg = map.is_wall(x, y) ? lightWall : scent_color;
    } else {
        tile.bg =
            map.is_wall(x, y) || !map.is_explored(x, y) ? darkWall : darkGround;
    }
}

void tcodRenderer::render_ant(LayoutBox const &box, Map &map, EntityData &a,
                              MapWindow const &window) {
    // SPDLOG_TRACE("Rendering ant at ({}, {})", a.x, a.y);
    long x, y;
    bool is_valid;
    window.to_local_coords(a.x, a.y, x, y, is_valid);
    if(!is_valid) {
        // SPDLOG_ERROR("Invalid coordinates ({}, {}) when rendering ant", a.x,
        // a.y);
        return;
    }

    RenderPosition &last_pos = a.last_rendered_pos;
    if(last_pos.requires_update) {
        // SPDLOG_TRACE("Clearing last position for ant at ({}, {})",
        // last_pos.x, last_pos.y);
        last_pos.requires_update = false;

        long last_x, last_y;
        window.to_local_coords(last_pos.x, last_pos.y, last_x, last_y,
                               is_valid);
        if(!is_valid) {
            // SPDLOG_ERROR("Invalid coordinates ({}, {}) when clearing last
            // position for ant", last_pos.x, last_pos.y);
            return;
        }

        clear_tile(box, last_x, last_y);
    }

    if(map.in_fov(a.x, a.y)) {
        // SPDLOG_TRACE("Rendering ant in FOV at ({}, {})", a.x, a.y);
        auto &tile = get_tile(box, x, y);
        tile.ch = a.ch;
        tile.fg = a.col;
        last_pos.x = a.x;
        last_pos.y = a.y;
    } else {
        // SPDLOG_ERROR("Ant at ({}, {}) is not in FOV", a.x, a.y);
    }
    // SPDLOG_TRACE("EntityData rendered");
}

void tcodRenderer::render_building(LayoutBox const &box, Building &b,
                                   MapWindow const &window) {
    // SPDLOG_TRACE("Rendering building at ({}, {})", b.x, b.y);
    for(long xi = b.border.x1; xi <= b.border.x2; ++xi) {
        for(long yi = b.border.y1; yi <= b.border.y2; ++yi) {
            long x, y;
            bool is_valid;
            window.to_local_coords(xi, yi, x, y, is_valid);
            if(!is_valid) continue;

            auto &tile = get_tile(box, x, y);
            tile.bg = b.color;
        }
    }
    // SPDLOG_TRACE("Building rendered");
}

void tcodRenderer::render_text_editor(LayoutBox const &box,
                                      TextEditor const &editor,
                                      ulong ant_count) {
    // SPDLOG_TRACE("Rendering text editor");
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

    mainBox.populate(editor.lines, editor.get_offset_x(),
                     editor.get_offset_y());
    accBox.populate({"ACC:0   "});
    bacBox.populate({"BAC:1   "});

    int numAntsSpaces =
        4 - static_cast<int>(std::to_string(ant_count).length());
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
        get_rect(box, editor.get_cursor_x() + 1 - editor.get_offset_x(),
                 editor.get_cursor_y() + 1 - editor.get_offset_y(), 1, 1),
        " ", color::white, color::light_green, TCOD_LEFT, TCOD_BKGND_SET);
    // SPDLOG_TRACE("Text editor rendered");
}

// TODO: display potential key presses that could be helpful.
// For instance, when standing in a nursery, display keys to produce new
// workers. This could be replaced with something else in the future.
void tcodRenderer::render_help_boxes(LayoutBox const &) {}

void tcodRenderer::render_sidebar(LayoutBox const& box, SidebarMenu const& menu,
                                  ClockSpeed clock_speed) {
    long w = box.get_width();
    long h = box.get_height();
    if(w <= 3 || h <= 3) return;

    std::vector<std::string> asciiGrid(h);
    for(long i = 0; i < h; ++i) {
        asciiGrid[i] = std::string(w, ' ');
    }

    Box sidebarBox(asciiGrid, 1, 1, static_cast<int>(w - 2),
                   static_cast<int>(h - 2));

    std::vector<std::string> lines;
    auto is_active_speed = [clock_speed](SidebarMenuAction action) {
        switch(clock_speed) {
            case ClockSpeed::PAUSED:
                return action == SidebarMenuAction::CLOCK_PAUSE;
            case ClockSpeed::NORMAL:
                return action == SidebarMenuAction::CLOCK_PLAY;
            case ClockSpeed::FAST:
                return action == SidebarMenuAction::CLOCK_FAST_FORWARD;
            default:
                return false;
        }
    };
    const auto& items = menu.items();
    for(size_t i = 0; i < items.size(); ++i) {
        bool is_selected = !menu.is_submenu_open() &&
                           menu.selected_index() == i;
        bool is_open = menu.open_submenu_index().has_value() &&
                       menu.open_submenu_index().value() == i;

        std::string label = items[i].label;
        if(!items[i].children.empty()) {
            label += is_open ? " v" : " >";
        }

        lines.push_back(std::string(is_selected ? "> " : "  ") + label);

        if(is_open) {
            const auto& submenu = items[i].children;
            for(size_t j = 0; j < submenu.size(); ++j) {
                bool is_sub_selected = menu.submenu_selected_index() == j;
                std::string sub_label = submenu[j].label;
                std::string prefix = is_sub_selected ? "  > " : "    ";
                std::string indicator =
                    is_active_speed(submenu[j].action) ? "* " : "  ";
                lines.push_back(prefix + indicator + sub_label);
            }
        }
    }

    sidebarBox.populate(lines);

    std::string result =
        std::accumulate(asciiGrid.begin(), asciiGrid.end(), std::string(""));

    tcod::print_rect(root_console, get_rect(box, 0, 0, static_cast<int>(w),
                                           static_cast<int>(h)),
                     result, color::white, color::dark_grey, TCOD_LEFT,
                     TCOD_BKGND_SET);
}

void tcodRenderer::render_toggle_button(bool is_expanded) {
    long x = 0, y = 0, w = 0, h = 0;
    SidebarMenu::get_toggle_button_bounds(x, y, w, h);

    const tcod::ColorRGB bg =
        is_expanded ? color::dark_grey : color::blue;

    tcod::print_rect(root_console,
                     {static_cast<int>(x), static_cast<int>(y),
                      static_cast<int>(w), static_cast<int>(h)},
                     "[M]", color::white, bg, TCOD_LEFT, TCOD_BKGND_SET);
}

void tcodRenderer::present() {
    // SPDLOG_TRACE("Presenting tcod context");
    context.present(root_console);
    // SPDLOG_TRACE("Presented tcod context");
}

void tcodRenderer::pixel_to_tile_coordinates(int pixel_x, int pixel_y,
                                             long &tile_x, long &tile_y) {
    std::array<int, 2> tile =
        context.pixel_to_tile_coordinates(std::array<int, 2>{pixel_x, pixel_y});
    tile_x = tile[0];
    tile_y = tile[1];
}

TCOD_ConsoleTile &tcodRenderer::clear_tile(LayoutBox const &box, long x,
                                           long y) {
    // SPDLOG_TRACE("Clearing tile at ({}, {})", x, y);
    auto &tile = get_tile(box, x, y);
    tile.ch = ' ';
    return tile;
}

TCOD_ConsoleTile &tcodRenderer::get_tile(LayoutBox const &box, long x, long y) {
    // SPDLOG_TRACE("Getting tile at ({}, {})", x, y);
    long abs_x = 0, abs_y = 0;
    box.get_abs_pos(x, y, abs_x, abs_y);
    // SPDLOG_TRACE("Getting tile at ({}, {}) -> abs ({}, {})", x, y, abs_x,
    // abs_y);
    return root_console.at(abs_x, abs_y);
}

const std::array<int, 4> tcodRenderer::get_rect(LayoutBox const &box, long x,
                                                long y, int w, int h) {
    // SPDLOG_TRACE("Getting rect at ({}, {}) with dimensions {}x{}", x, y, w,
    // h);
    long abs_x = 0, abs_y = 0;
    box.get_abs_pos(x, y, abs_x, abs_y);
    return {(int)abs_x, (int)abs_y, w, h};
}

void tcodRenderer::use_default_tile_rendering() {
    SPDLOG_INFO("Using default map tile renderer");
    generate_tile_renderer = [](Map &map) {
        return std::make_unique<TcodMapTileRenderer>(map);
    };
}

void tcodRenderer::use_debug_tile_rendering() {
    SPDLOG_INFO("Using default map tile renderer");
    generate_tile_renderer = [](Map &map) {
        return std::make_unique<DebugMapTileRenderer>(map);
    };
}

void tcodRenderer::use_scent_tile_rendering(ulong scent_idx) {
    SPDLOG_INFO("Using scent map tile renderer - scent index: {}", scent_idx);
    generate_tile_renderer = [scent_idx](Map &map) {
        return std::make_unique<ScentMapTileRenderer>(map, scent_idx);
    };
}
