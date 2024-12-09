#include "utils/algorithm.hpp"
#include <libtcod/mersenne.hpp>

using namespace algorithm;

bool algorithm::can_place_shape(const grid_t& grid, int x, int y, const grid_t& shape) {
    ulong shape_width = shape[0].size();
    ulong shape_height = shape.size();
    for(ulong i = x, si = 0; i < x + shape_width; ++i, ++si) {
        for(ulong j = y, sj = 0; j < y + shape_height; ++j, ++sj) {
            if(i >= grid.size() || j >= grid[0].size() || (grid[i][j] != nullptr && shape[si][sj] != nullptr) ) {
                return false;
            }
        }
    }
    return true;
}


void algorithm::place_rectangle(grid_t& grid, int x, int y, const grid_t& shape) {
    ulong shape_width = shape[0].size();
    ulong shape_height = shape.size();
    for(ulong i = x, si = 0; i < x + shape_width; ++i, ++si) {
        for(ulong j = y, sj = 0; j < y + shape_height; ++j, ++sj) {
            grid[i][j] = shape[si][sj];
        }
    }
}


grid_t algorithm::best_fit_packing(std::vector<grid_t>& shapes, int gridSize, TCODRandom& randomizer) {
    grid_t grid(gridSize, std::vector<void*>(gridSize, 0));
    std::shuffle(shapes.begin(), shapes.end(), randomizer);

    for( const auto& shape: shapes) {
        bool placed = false;
        ulong shape_width = shape[0].size();
        ulong shape_height = shape.size();
        for(ulong x = 0; x <= gridSize - shape_width; ++x) {
            for(ulong y = 0; y <= gridSize - shape_height; ++y) {
                if(can_place_shape( grid, x, y, shape ) ) {
                    place_rectangle( grid, x, y, shape );
                    placed = true;
                    break;
                }
            }
            if(placed) {
                break;
            }
        }
    }
    return grid;
}

