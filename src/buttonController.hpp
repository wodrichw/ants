#pragma once
#include <libtcod/color.hpp>
#include <optional>
#include <unordered_set>
#include "globals.hpp"

// Button Controller manages all the buttons that are present. The
// buttons are managed in layers so that if a mouse clicks on an
// are where two or more buttons occupy, the click will trigger
// the top most layer as described in the Layer_manager, then
// the button callback can choose to pass on the trigger to
// the remaining buttons that occupy that space or not.
class ButtonController {
public:

    // buttons layer enum which is used to describe which layer a button belongs to
    enum Layer {
        FIRST=0, SECOND, THIRD, FOURTH, FIFTH
    };

    struct Button {
        size_t x, y, w, h;
        Layer layer;
        // Todo: make a ClickEvent struct which holds information about the button click.
        // For instance, it can hold information like click duration and maybe which mouse
        // button clicked the button.
        // If onClick returns true -> call buttons that overlap the click location on lower layers
        // If onClick returns false-> don't call any more button onClick functions
        std::function<bool()>onClick;

        std::optional<tcod::ColorRGB> color;
    };


private:
    // butons is a 2D array which maps a (layer, x, y) coordinate to a Button. 
    // This way, when the engine gets a mouse event, it can see what buttons occupy
    // that x, y location and call the corresponding Button::onClick functions
    std::array<std::array<Button*, globals::COLS*globals::ROWS>, globals::NUM_BUTTON_LAYERS> buttons;

    // buttonTracker keeps track of all the buttons that are currently managed by the
    // ButtonController. This makes it easier to iterate over all the buttons
    std::unordered_set<Button*> buttonTracker;

public:
    ButtonController();
    ~ButtonController();

    // returns true if button successfully added, false if otherwise
    // will not add a button to a location if it is already occupied 
    // by another button on the same (layer, x,y) space
    bool addButton(Button* b);
    void removeButton(Button* b);
    void handleClick(size_t x, size_t y);
    bool canMoveButton(Button* b, int dx, int dy);
    void moveButton(Button* b, int dx, int dy);

private:
    inline size_t idx(size_t x, size_t y) const { return x+(y*globals::COLS); }
};
