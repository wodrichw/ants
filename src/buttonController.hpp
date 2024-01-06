#pragma once

#include <libtcod/color.hpp>
#include <optional>

// Button Controller manages all the buttons that are present. The
// buttons are managed in layers so that if a mouse clicks on an
// are where two or more buttons occupy, the click will trigger
// the top most layer as described in the Layer_manager, then
// the button callback can choose to pass on the trigger to
// the remaining buttons that occupy that space or not.
class ButtonController {

    struct Button {
        size_t x, y, w, h;
        std::optional<tcod::ColorRGB> color;
    };
};
