#ifndef __COLORS_HPP
#define __COLORS_HPP

#include <libtcod/color.hpp>

namespace color {
    static const tcod::ColorRGB white        {255,255,255};
    static const tcod::ColorRGB grey         {97,92,112};
    static const tcod::ColorRGB dark_grey    {58,55,67};
    static const tcod::ColorRGB light_black  {23, 18, 23};
    static const tcod::ColorRGB black        {0,0,0};

    static const tcod::ColorRGB red          {235,81,96};
    static const tcod::ColorRGB indian_red   {193, 102, 107};
    static const tcod::ColorRGB dark_yellow  {242,158,24};
    static const tcod::ColorRGB light_green  {68,207,108};
    static const tcod::ColorRGB blue         {40,163,204};
    static const tcod::ColorRGB dark_blue    {25,100,126};
    static const tcod::ColorRGB violet       {180,126,179};
}

#endif // __COLORS_HPP
