#include "buttonController.hpp"
#include "globals.hpp"

ButtonController::ButtonController():
    buttons()
{
    for(size_t l = 0; l < globals::NUM_BUTTON_LAYERS; ++l) {
        buttons[l].fill(nullptr);
    }
}

ButtonController::~ButtonController()
{
    for( Button* b: buttonTracker ) {
        delete b;
    }
}


ButtonController::Button* ButtonController::createButton(long x, long y, long w, long h, Layer layer, std::function<bool()>onClick, std::optional<tcod::ColorRGB> color)
{
    long end_x = x + w, end_y = y + h;
    // check if button is in game grid
    if( (end_x) > globals::COLS ) return nullptr;
    if( (end_y) > globals::ROWS ) return nullptr;

    // check if the space is already occupied by a button
    for( long xi = x; xi < end_x; ++xi ) {
        for( long yi = y; yi < end_y; ++yi ) {
            if( buttons[layer][xi + yi * globals::COLS ] ) return nullptr;
        }
    }

    Button *b = new ButtonController::Button{x, y, w, h, layer, onClick, color};

    // At this point it is safe to assume that the button can be added
    for( long xi = b->x; xi < end_x; ++xi ) {
        for( long yi = y; yi < end_y; ++yi ) {
            buttons[b->layer][xi+(yi*globals::COLS)] = b;
        }
    }
    buttonTracker.insert(b);
    return b;
}

void ButtonController::removeButton(Button* b)
{
    for( long xi = b->x; xi < b->x + b->w; ++xi ) {
        for( long yi = b->y; yi < b->w + b->h; ++yi ) {
            buttons[b->layer][idx(xi, yi)] = nullptr;
        }
    }
    buttonTracker.erase(b);
    delete b;
}

void ButtonController::handleClick(long x, long y)
{
    for( int layer = globals::NUM_BUTTON_LAYERS - 1; layer >= 0; --layer ) {
        bool continueToLowerLevelButtons = true;
        Button* b =  buttons[layer][idx(x,y)];
        if( b ) {
            continueToLowerLevelButtons = b->onClick();
        }
        if( !continueToLowerLevelButtons ) break;
    }
}

bool ButtonController::canMoveButton(Layer layer, long x, long y, long w, long h)
{
    if( x < 0 || x >=  globals::COLS ) return false;
    if( y < 0 || y >=  globals::ROWS ) return false;
    for( long xi = x; xi < x+w; ++xi ) {
        for( long yi = y; yi < y+h; ++yi ) {
            if( buttons[layer][idx(xi,yi)] ) return false;
        }
    }
    return true;
}

bool ButtonController::canMoveButton(ButtonController::Button* b, long dx, long dy)
{
    return canMoveButton(b->layer, b->x + dx, b->y + dy, b->w, b->h);
}

void ButtonController::moveButton(Button* b, long dx, long dy)
{
    // remove button from old square
    for( long xi = b->x; xi < b->x+b->w; ++xi ) {
        for( long yi = b->y; yi < b->y+b->h; ++yi ) {
            buttons[b->layer][idx(xi, yi)] = nullptr;
        }
    }
    // add button to new square
    int new_x = b->x + dx;
    int new_y = b->y + dy;
    for( long xi = new_x; xi < new_x+b->w; ++xi ) {
        for( long yi = new_y; yi < new_y+b->h; ++yi ) {
            buttons[b->layer][idx(xi,yi)] = b;
        }
    }
    // update button's internal position
    b->x += dx;
    b->y += dy;
}

