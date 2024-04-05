#include "buttonController.hpp"

#include "globals.hpp"

ButtonController::ButtonController() : buttons() {
    for(size_t l = 0; l < globals::NUM_BUTTON_LAYERS; ++l) {
        buttons[l].fill(nullptr);
    }
}

ButtonController::~ButtonController() {
    for(Button* b : buttonTracker) {
        delete b;
    }
}

bool ButtonController::canCreateButton(
    ButtonController::ButtonData const& data) {
    long end_x = data.x + data.w, end_y = data.y + data.h;
    // check if button is in game grid
    if((end_x) > globals::COLS) return false;
    if((end_y) > globals::ROWS) return false;

    // check if the space is already occupied by a button
    for(long xi = data.x; xi < end_x; ++xi) {
        for(long yi = data.y; yi < end_y; ++yi) {
            if(buttons[data.layer][xi + yi * globals::COLS]) return false;
        }
    }
    return true;
}

ButtonController::Button* ButtonController::createButton(
    ButtonData const& data, std::function<bool()> onClick,
    std::optional<tcod::ColorRGB> color) {
    long end_x = data.x + data.w, end_y = data.y + data.h;
    Button* b = new ButtonController::Button{data, onClick, color};

    // At this point it is safe to assume that the button can be added
    for(long xi = b->data.x; xi < end_x; ++xi) {
        for(long yi = data.y; yi < end_y; ++yi) {
            buttons[b->data.layer][xi + (yi * globals::COLS)] = b;
        }
    }
    buttonTracker.insert(b);
    return b;
}

void ButtonController::removeButton(Button* b) {
    for(long xi = b->data.x; xi < b->data.x + b->data.w; ++xi) {
        for(long yi = b->data.y; yi < b->data.w + b->data.h; ++yi) {
            buttons[b->data.layer][idx(xi, yi)] = nullptr;
        }
    }
    buttonTracker.erase(b);
    delete b;
}

void ButtonController::handleClick(long x, long y) {
    for(int layer = globals::NUM_BUTTON_LAYERS - 1; layer >= 0; --layer) {
        bool continueToLowerLevelButtons = true;
        Button* b = buttons[layer][idx(x, y)];
        if(b) {
            continueToLowerLevelButtons = b->onClick();
        }
        if(!continueToLowerLevelButtons) break;
    }
}

bool ButtonController::canMoveButton(Layer layer, long x, long y, long w,
                                     long h) {
    if(x < 0 || x >= globals::COLS) return false;
    if(y < 0 || y >= globals::ROWS) return false;
    for(long xi = x; xi < x + w; ++xi) {
        for(long yi = y; yi < y + h; ++yi) {
            if(buttons[layer][idx(xi, yi)]) return false;
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
    for(long xi = b->data.x; xi < b->data.x + b->data.w; ++xi) {
        for(long yi = b->data.y; yi < b->data.y + b->data.h; ++yi) {
            buttons[b->data.layer][idx(xi, yi)] = nullptr;
        }
    }
    // add button to new square
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
}
