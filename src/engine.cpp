#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <SDL_keycode.h>
#include <cassert>
#include <cstdlib>
#include <libtcod.hpp>
#include <libtcod/console.hpp>
#include <libtcod/context.h>
#include <libtcod/context.hpp>
#include <libtcod/context_init.h>
#include <numeric>
#include <iostream>

#include "ant.hpp"
#include "colors.hpp"
#include "controller.hpp"
#include "engine.hpp"

static const int ROWS = 60;
static const int COLS = 60;

Engine::Engine()
    : player(new ant::Player(40, 25, 10, '@', color::white)), ants({player}),
    buildings(), controllers(), map(new Map(COLS, ROWS, ants, buildings)),
    clock_timeout_1000ms(SDL_GetTicks64()), textEditorLines(textBoxHeight) {
        gameStatus = STARTUP;
        auto params = TCOD_ContextParams();
        params.columns = COLS, params.rows = ROWS, params.window_title = "A N T S";
        context = tcod::Context(params);
        map->root_console = context.new_console(COLS, ROWS);

        for (int i = 0; i < textBoxHeight; ++i) {
            textEditorLines[i] = std::string(textBoxWidth, ' ');
        }
    }

Engine::~Engine() {
    for (auto ant : ants)
        delete ant;
    delete map;
}

struct Box {
    ulong x, y, w, h;
    std::vector<std::string> &asciiGrid;
    Box(std::vector<std::string> &asciiGrid, int x, int y, int w, int h)
        : x(x), y(y), w(w), h(h), asciiGrid(asciiGrid) {}

    void populateChar(int x_idx, int y_idx, char ch) {
        asciiGrid[y_idx + y][x + x_idx] = ch;
    }

    void checkInputText(const std::vector<std::string> &text) {
        assert(text.size() == h - 2);
        assert(
                std::all_of(text.begin(), text.end(), [this](const std::string &str) {
                    return str.length() == w - 2;
                    }));
    }

    void populate(const std::vector<std::string> &text) {
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

struct Column {
    int width, height;
};

void Engine::printTextEditor() {
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
void Engine::printHelpBoxes() {}

void Engine::moveToPrevNonWhiteSpace() {
    while (cursorX > 0 && textEditorLines[cursorY][cursorX - 1] == ' ')
        --cursorX;
}

void Engine::moveToEndLine() {
    cursorX = textBoxWidth - 1;
    moveToPrevNonWhiteSpace();
}

void Engine::handleTextEditorAction(SDL_Keycode key_sym) {
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
    } else if (key_sym >= SDLK_a && key_sym <= SDLK_z &&
            cursorX < (textBoxWidth - 1)) {
        textEditorLines[cursorY][cursorX] = toupper(key_sym);
        cursorX++;
    } else if (((key_sym >= SDLK_0 && key_sym <= SDLK_9) ||
                key_sym == SDLK_COMMA || key_sym == SDLK_SPACE) &&
            cursorX < (textBoxWidth - 1)) {
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

void Engine::handleKeyPress(SDL_Keycode key_sym, int &dx, int &dy) {
    if (key_sym == SDLK_SLASH && gameStatus == TEXT_EDITOR) {
        gameStatus = IDLE;
        return;
    }

    if (key_sym == SDLK_SLASH && gameStatus != TEXT_EDITOR) {
        gameStatus = TEXT_EDITOR;
        return;
    }

    if (gameStatus == TEXT_EDITOR) {
        handleTextEditorAction(key_sym);
        return;
    }

    if (key_sym == SDLK_a && player->bldgId.has_value()) {
        // Make worker
        // TODO: make an intelligent location picker for workers (addAnt_x,addAnt_y)
        //   -- should look around building and scan for an open location
        //   -- if no location present increase the scan radius (as if the building
        //   absorbs its perimiter squares)
        //      and look for open squared there. Radius increasing will go on until
        //      an open square is found (or out of space in the map)

        Building &b = *buildings[player->bldgId.value()]; // TODO: restrict to only nursersies
        int addAnt_x = b.x, addAnt_y = b.y;
        ant::Worker* new_ant = new ant::Worker(addAnt_x, addAnt_y);

        EngineInteractor interactor;
        interactor.move_ant = [&, new_ant](int dx, int dy) {
            if (map->canWalk(new_ant->x + dx, new_ant->y + dy))
                moveAnt(new_ant, dx, dy);
        };

        Worker_Controller* w = new Worker_Controller(assembler, interactor,textEditorLines);

        if (! interactor.status.p_err ) { // add worker ant if the textEditorLines have no parser errors
            controllers.push_back(w);
            ants.push_back(new_ant);
        } else {
            delete w;
            delete new_ant;
            // TODO: show parse errors in the text editor box instead of a cout
            // this will likely require returning the line number, and word that caused the parse error
            std::cout << interactor.status.err_msg << std::endl;
        }

        return;
    }

    // update player location input (only if no other keys were pressed)
    if (key_sym == SDLK_h || key_sym == SDLK_LEFT) {
        dx = -1;
    } else if (key_sym == SDLK_j || key_sym == SDLK_DOWN) {
        dy = 1;
    } else if (key_sym == SDLK_k || key_sym == SDLK_UP) {
        dy = -1;
    } else if (key_sym == SDLK_l || key_sym == SDLK_RIGHT) {
        dx = 1;
    }
}

void Engine::moveAnt(ant::Ant *ant, int dx, int dy) {
    map->clearCh(ant->x, ant->y);
    ant->updatePositionByDelta(dx, dy);
    map->updateFov();

    if (map->getTile(ant->x, ant->y).bldgId.has_value()) {
        ant->bldgId.emplace(map->getTile(ant->x, ant->y).bldgId.value());
    } else {
        ant->bldgId.reset();
    }
}

void Engine::update() {
    if (gameStatus == STARTUP) {
        map->updateFov();
        render();
    }
    if (gameStatus != TEXT_EDITOR)
        gameStatus = IDLE;

    SDL_Event event;
    int dx = 0, dy = 0;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                std::exit(EXIT_SUCCESS);
            case SDL_KEYDOWN:
                handleKeyPress(event.key.keysym.sym, dx, dy);
                break;
        }
    }


    if (clock_timeout_1000ms < SDL_GetTicks64()) {
        for (Controller *c : controllers) {
            c->handleClockPulse();
        }
        clock_timeout_1000ms += 1000;
    }

    if ((dx != 0 || dy != 0) && map->canWalk(player->x + dx, player->y + dy)) {
        moveAnt(player, dx, dy);
    }
}

void Engine::render() {
    // draw the map
    map->render();

    // draw the ants
    for (auto ant : ants) {
        map->renderAnt(*ant);
    }

    // draw the buildings
    for (auto building : buildings) {
        map->renderBuilding(*building);
    }

    if (gameStatus == TEXT_EDITOR)
        printTextEditor();

    printHelpBoxes();

    context.present(map->root_console);
}
