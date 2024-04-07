#include "buttonController.hpp"

#include "globals.hpp"
#include "spdlog/spdlog.h"

ButtonController::ButtonController() : buttons() {
    SPDLOG_DEBUG("ButtonController created - button layers: {}",
                 globals::NUM_BUTTON_LAYERS);
    for(size_t l = 0; l < globals::NUM_BUTTON_LAYERS; ++l) {
        buttons[l].fill(nullptr);
    }
    SPDLOG_TRACE("ButtonController initialized");
}

ButtonController::~ButtonController() {
    SPDLOG_DEBUG("Destructing ButtonController");
    for(Button* b : buttonTracker) {
        delete b;
    }
    SPDLOG_TRACE("ButtonController destructed");
}

bool ButtonController::canCreateButton(
    ButtonController::ButtonData const& data) {
    SPDLOG_TRACE("Checking if button can be created at ({}, {}, {}, {})", data.x, data.y, data.w, data.h);
    long end_x = data.x + data.w, end_y = data.y + data.h;
    // check if button is in game grid
    if((end_x) > globals::COLS) {
        SPDLOG_DEBUG("Button is out of bounds on the x-axis");
        return false;
    }
    if((end_y) > globals::ROWS) {
        SPDLOG_DEBUG("Button is out of bounds on the y-axis");
        return false;
    }

    // check if the space is already occupied by a button
    SPDLOG_TRACE("Checking if button is already occupied");
    for(long xi = data.x; xi < end_x; ++xi) {
        for(long yi = data.y; yi < end_y; ++yi) {
            if(buttons[data.layer][xi + yi * globals::COLS]) {
                SPDLOG_DEBUG("Button is already occupied at ({}, {})", xi, yi);
                return false;
            }
        }
    }
    SPDLOG_TRACE("Button can be created");
    return true;
}

ButtonController::Button* ButtonController::createButton(
    ButtonData const& data, std::function<bool()> onClick,
    std::optional<tcod::ColorRGB> color) {
    SPDLOG_DEBUG("Creating button at ({}, {}, {}, {})", data.x, data.y, data.w, data.h);

    long end_x = data.x + data.w, end_y = data.y + data.h;
    Button* b = new ButtonController::Button{data, onClick, color};

    // At this point it is safe to assume that the button can be added
    SPDLOG_DEBUG("Button created at ({}, {}, {}, {})", data.x, data.y, data.w, data.h);
    for(long xi = b->data.x; xi < end_x; ++xi) {
        for(long yi = data.y; yi < end_y; ++yi) {
            buttons[b->data.layer][xi + (yi * globals::COLS)] = b;
        }
    }
    buttonTracker.insert(b);
    SPDLOG_TRACE("Button added to buttonTracker - creation completed");
    return b;
}

void ButtonController::removeButton(Button* b) {
    SPDLOG_DEBUG("Removing button at ({}, {}, {}, {})", b->data.x, b->data.y, b->data.w, b->data.h);
    for(long xi = b->data.x; xi < b->data.x + b->data.w; ++xi) {
        for(long yi = b->data.y; yi < b->data.w + b->data.h; ++yi) {
            buttons[b->data.layer][idx(xi, yi)] = nullptr;
        }
    }
    buttonTracker.erase(b);
    delete b;
    SPDLOG_TRACE("Button removed");
}

void ButtonController::handleClick(long x, long y) {
    SPDLOG_DEBUG("Handling click at ({}, {})", x, y);
    for(int layer = globals::NUM_BUTTON_LAYERS - 1; layer >= 0; --layer) {
        SPDLOG_TRACE("Checking layer {}", layer);
        bool continueToLowerLevelButtons = true;
        Button* b = buttons[layer][idx(x, y)];
        if(b) {
            continueToLowerLevelButtons = b->onClick();
        }
        if(!continueToLowerLevelButtons) break;
    }
    SPDLOG_TRACE("Click on button controller handled");
}

bool ButtonController::canMoveButton(Layer layer, long x, long y, long w,
                                     long h) {
    SPDLOG_TRACE("Checking if button can be moved to ({}, {}, {}, {})", x, y, w, h);
    if(x < 0 || x >= globals::COLS) {
        SPDLOG_DEBUG("Button is out of bounds on the x-axis");
        return false;
    }
    if(y < 0 || y >= globals::ROWS) {
        SPDLOG_DEBUG("Button is out of bounds on the y-axis");
        return false;
    }

    SPDLOG_TRACE("Checking if button is already occupied");
    for(long xi = x; xi < x + w; ++xi) {
        for(long yi = y; yi < y + h; ++yi) {
            if(buttons[layer][idx(xi, yi)]) {
                SPDLOG_DEBUG("Button is already occupied at ({}, {})", xi, yi);
                return false;
            }
        }
    }
    return true;
}

bool ButtonController::canMoveButton(ButtonController::Button* b, long dx,
                                     long dy) {
    return canMoveButton(b->data.layer, b->data.x + dx, b->data.y + dy,
                         b->data.w, b->data.h);
}

void ButtonController::moveButton(Button* b, long dx, long dy) {
    // remove button from old square
    SPDLOG_DEBUG("Moving button from ({}, {}) to ({}, {})", b->data.x, b->data.y, b->data.x + dx, b->data.y + dy);
    for(long xi = b->data.x; xi < b->data.x + b->data.w; ++xi) {
        for(long yi = b->data.y; yi < b->data.y + b->data.h; ++yi) {
            buttons[b->data.layer][idx(xi, yi)] = nullptr;
        }
    }

    // add button to new square
    SPDLOG_DEBUG("Adding button to new location");
    int new_x = b->data.x + dx;
    int new_y = b->data.y + dy;
    for(long xi = new_x; xi < new_x + b->data.w; ++xi) {
        for(long yi = new_y; yi < new_y + b->data.h; ++yi) {
            buttons[b->data.layer][idx(xi, yi)] = b;
        }
    }

    // update button's internal position
    b->data.x += dx;
    b->data.y += dy;
    SPDLOG_TRACE("Button moved to ({}, {})", b->data.x, b->data.y);
}
