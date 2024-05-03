#pragma once

#ifdef DBG_GRAPHICS
#include "ui/debug_graphics_include.hpp"
#define DEBUG_CHUNKS(box, map, window, get_tile, is_debug_graphics) \
    debug_chunks(box, map, window, get_tile, is_debug_graphics)
#else
#define DEBUG_CHUNKS(box, map, window, get_tile, is_debug_graphics)
#endif
