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


bool ButtonController::addButton(Button* b)
{
    // check if button is in game grid
    if( (b->x+b->w) > globals::COLS ) return false;
    if( (b->y+b->h) > globals::ROWS ) return false;

    // check if the space is already occupied by a button 
    for( size_t xi = b->x; xi < b->x + b->w; ++xi ) {
        for( size_t yi = b->y; yi < b->y + b->h; ++yi ) {
            if( buttons[b->layer][xi*yi] ) return false;
        }
    }


    // At this point it is safe to assume that the button can be added
    for( size_t xi = b->x; xi < b->x + b->w; ++xi ) {
        for( size_t yi = b->y; yi < b->y + b->h; ++yi ) {
            buttons[b->layer][xi+(yi*globals::COLS)] = b;
        }
    }
    buttonTracker.insert(b);
    return true;
}

void ButtonController::removeButton(Button* b)
{
    for( size_t xi = b->x; xi < b->x + b->w; ++xi ) {
        for( size_t yi = b->y; yi < b->w + b->h; ++yi ) {
            buttons[b->layer][idx(xi, yi)] = nullptr;
        }
    }
    buttonTracker.erase(b);
    delete b;
}

void ButtonController::handleClick(size_t x, size_t y)
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

bool ButtonController::canMoveButton(Button* b, int dx, int dy)
{
    int new_x = b->x + dx;
    int new_y = b->y + dy;
    if( new_x >=  globals::COLS || new_x < 0 ) return false;
    if( new_y >=  globals::ROWS || new_y < 0 ) return false;
    for( size_t xi = new_x; xi < new_x+b->w; ++xi ) {
        for( size_t yi = new_y; yi < new_y+b->h; ++yi ) {
            if( buttons[b->layer][idx(xi,yi)] ) return false;
        }
    }
    return true;
}

void ButtonController::moveButton(Button* b, int dx, int dy)
{
    // remove button from old square
    for( size_t xi = b->x; xi < b->x+b->w; ++xi ) {
        for( size_t yi = b->y; yi < b->y+b->h; ++yi ) {
            buttons[b->layer][idx(xi, yi)] = nullptr;
        }
    }
    // add button to new square
    int new_x = b->x + dx;
    int new_y = b->y + dy;
    for( size_t xi = new_x; xi < new_x+b->w; ++xi ) {
        for( size_t yi = new_y; yi < new_y+b->h; ++yi ) {
            buttons[b->layer][idx(xi,yi)] = b;
        }
    }
    // update button's internal position
    b->x += dx;
    b->y += dy;
}

