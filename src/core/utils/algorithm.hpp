#pragma once

#include <libtcod/mersenne.hpp>
#include <vector>  
namespace algorithm {

using grid_t = std::vector<std::vector<void*>>;
struct Rectangle {  
    int width;  
    int height;  
};  
struct Position {  
    int x;  
    int y;  
};
bool can_place_shape(const grid_t& grid, int x, int y, const grid_t& shape);
void place_rectangle(grid_t& grid, int x, int y, const grid_t& shape);
grid_t best_fit_packing(std::vector<grid_t>& shapes, int gridSize, TCODRandom& randomizer);
}
