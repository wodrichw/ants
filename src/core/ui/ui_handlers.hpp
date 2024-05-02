#pragma once

#include "entity/map.hpp"
#include "ui/render.hpp"
#include "ui/subscriber.hpp"
#include "entity/entity_manager.hpp"

class MoveLeftHandler: public Subscriber<KeyboardEvent> {
    Map& map;
    MapEntity& entity;
    bool& text_editor_active;
public:
    MoveLeftHandler(Map& map, MapEntity& entity, bool& text_editor_active):
        map(map),
        entity(entity),
        text_editor_active(text_editor_active)
    {}

    void operator()(KeyboardEvent const&) { 
        if( text_editor_active ) return;
        map.move_entity(entity, -1, 0);
        map.need_update_fov = true;
    }
};

class MoveRightHandler: public Subscriber<KeyboardEvent> {
    Map& map;
    MapEntity& entity;
    bool& text_editor_active;
public:
    MoveRightHandler(Map& map, MapEntity& entity, bool& text_editor_active):
        map(map),
        entity(entity),
        text_editor_active(text_editor_active)
    {}

    void operator()(KeyboardEvent const&) {
        if( text_editor_active ) return;
        map.move_entity(entity, 1, 0);
        map.need_update_fov = true;
    }
};

class MoveUpHandler: public Subscriber<KeyboardEvent> {
    Map& map;
    MapEntity& entity;
    bool& text_editor_active;
public:
    MoveUpHandler(Map& map, MapEntity& entity, bool& text_editor_active):
        map(map),
        entity(entity),
        text_editor_active(text_editor_active)
    {}

    void operator()(KeyboardEvent const&) {
        if( text_editor_active ) return;
        map.move_entity(entity, 0, -1);
        map.need_update_fov = true;
    }
};

class MoveDownHandler: public Subscriber<KeyboardEvent> {
    Map& map;
    MapEntity& entity;
    bool& text_editor_active;
public:
    MoveDownHandler(Map& map, MapEntity& entity, bool& text_editor_active):
        map(map),
        entity(entity),
        text_editor_active(text_editor_active)
    {}

    void operator()(KeyboardEvent const&) {
        if( text_editor_active ) return;
        map.move_entity(entity, 0, 1);
        map.need_update_fov = true;
    }
};

class ClickHandler: public Subscriber<MouseEvent> {
    Map& map;
    Renderer& renderer;
    bool& text_editor_active;
public:
    ClickHandler(Map& map, Renderer& renderer, bool& text_editor_active):
        map(map),
        renderer(renderer),
        text_editor_active(text_editor_active)
    {}

    void operator()(MouseEvent const& event) { 
        if( text_editor_active ) return;
        long x = 0, y = 0;
        renderer.pixel_to_tile_coordinates(event.x, event.y, x, y);
        map.click(x, y);
    }
};

class CreateAntHandler: public Subscriber<KeyboardEvent> {
    EntityManager& entity_manager;
    std::vector<ClockController*>& controllers;
    TextEditor& editor;
public:
    CreateAntHandler(EntityManager& entity_manager, std::vector<ClockController*>& controllers, TextEditor& editor):
        entity_manager(entity_manager), controllers(controllers), editor(editor) {}
    void operator()(KeyboardEvent const&) { 
        if( editor.is_active ) return;
        entity_manager.create_ant(controllers, editor.lines);
    }
};

