#include <libtcod.hpp>
#include "ant.hpp"
#include "building.hpp"
#include "text_editor_handler.hpp"

class tcodRenderer {
    public:
    tcodRenderer();
    void renderMap(Map& map);
    void renderAnt(Map& map, ant::Ant& a);
    void renderBuilding(Building& b);
    void renderTextEditor(TextEditorHandler const& editor);
    void renderHelpBoxes();
    void present();
    void pixel_to_tile_coordinates(int pixel_x, int pixel_y, size_t& tile_x, size_t& tile_y);

private:
    void clearCh(int x, int y);

    tcod::Context context;
    tcod::Console root_console;

    int fovRadius = 10;
    bool computeFov = true;
};
