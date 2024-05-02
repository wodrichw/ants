#pragma once

#include <vector>
#include <SDL2/SDL_timer.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <libtcod.hpp>
#include <libtcod/console.hpp>

#include "ui/render.hpp"
#include "ui/text_editor_handler.hpp"
#include "ui/event_system.hpp"
#include "entity/entity_manager.hpp"
#include "entity/map_entity.hpp"
#include "ui/ui_handlers.hpp"

class Mode {
    public:
    virtual bool is_editor() = 0;
    virtual bool is_primary() = 0;
    virtual void render() = 0;
    virtual void update() = 0;
    virtual EventPublisher<MouseEventType, MouseEvent>& get_mouse_publisher() = 0;
    virtual EventPublisher<KeyboardEventType, KeyboardEvent>& get_keyboard_publisher() = 0;
    virtual EventPublisher<CharKeyboardEventType, CharKeyboardEvent>& get_char_keyboard_publisher() = 0;
};

class EditorMode : public Mode {
    Renderer& renderer;
    LayoutBox& box;
    TextEditor& editor;
    std::vector<MapEntity*> const& ants;
    EventSystem event_system;

public:
    EditorMode(
        Renderer& renderer, LayoutBox& box, TextEditor& editor,
        std::vector<MapEntity*> const& ants): renderer(renderer), box(box), editor(editor), ants(ants) {
        
        // text editor listeners
        event_system.keyboard_events.add(RETURN_KEY_EVENT, new NewLineHandler(editor));
        event_system.keyboard_events.add(BACKSPACE_KEY_EVENT, new BackspaceHandler(editor));
        event_system.keyboard_events.add(LEFT_KEY_EVENT, new MoveCursorLeftHandler(editor));
        event_system.keyboard_events.add(RIGHT_KEY_EVENT, new MoveCursorRightHandler(editor));
        event_system.keyboard_events.add(UP_KEY_EVENT, new MoveCursorUpHandler(editor));
        event_system.keyboard_events.add(DOWN_KEY_EVENT, new MoveCursorDownHandler(editor));

        // text editor printable char listener
        event_system.char_keyboard_events.add(CHAR_KEY_EVENT, new EditorKeyHandler(editor));
    }
    
    bool is_editor() override { return true; }
    bool is_primary() override { return false; }

    void render() override {
        renderer.renderTextEditor(box, editor, ants.size());
    }

    void update() override {}
    
    EventPublisher<MouseEventType, MouseEvent>& get_mouse_publisher() override {
        return event_system.mouse_events;
    }

    EventPublisher<KeyboardEventType, KeyboardEvent>& get_keyboard_publisher() override {
        return event_system.keyboard_events;
    }

    EventPublisher<CharKeyboardEventType, CharKeyboardEvent>& get_char_keyboard_publisher() override {
        return event_system.char_keyboard_events;
    }
};

class PrimaryMode : public Mode {

    EventSystem event_system;
    LayoutBox& box;
    EntityManager& entity_manager;
    Renderer& renderer;
    std::vector<ClockController*> clockControllers;
    ulong clock_timeout_1000ms;

public:
    PrimaryMode(LayoutBox& box, EntityManager& entity_manager, Renderer& renderer, TextEditor& editor):
        box(box), entity_manager(entity_manager), renderer(renderer), clockControllers(),
        clock_timeout_1000ms(SDL_GetTicks64()) {
    
        event_system.keyboard_events.add(LEFT_KEY_EVENT, new MoveLeftHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(RIGHT_KEY_EVENT, new MoveRightHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(UP_KEY_EVENT, new MoveUpHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(DOWN_KEY_EVENT, new MoveDownHandler(entity_manager.map, entity_manager.player));

        event_system.keyboard_events.add(H_KEY_EVENT, new MoveLeftHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(L_KEY_EVENT, new MoveRightHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(K_KEY_EVENT, new MoveUpHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(J_KEY_EVENT, new MoveDownHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(A_KEY_EVENT, new CreateAntHandler(entity_manager, clockControllers, editor));

        // click listeners
        event_system.mouse_events.add(LEFT_MOUSE_EVENT, new ClickHandler(entity_manager.map, renderer));
    }

    bool is_editor() override { return false; }
    bool is_primary() override { return true; }

    void render() override {
        // SPDLOG_TRACE("Rendering engine");
        entity_manager.compute_fov();

        // draw the map
        renderer.renderMap(box, entity_manager.map);


        // draw the buildings
        // SPDLOG_TRACE("Rendering {} buildings", buildings.size());
        for(auto building : entity_manager.buildings) {
            renderer.renderBuilding(box, *building);
        }

        // draw the ants
        // SPDLOG_TRACE("Rendering {} ants", ants.size());
        for(auto ant : entity_manager.ants) {
            renderer.renderAnt(box, entity_manager.map, ant->get_data());
        }
    }

    void update() override {
        // SPDLOG_TRACE("Checking for clock pulse");
        if(clock_timeout_1000ms < SDL_GetTicks64()) {
            // SPDLOG_TRACE("Detected clock pulse");
            for(ClockController* c : clockControllers) {
                c->handleClockPulse();
            }
            clock_timeout_1000ms += 1000;
        }

    }

    EventPublisher<MouseEventType, MouseEvent>& get_mouse_publisher() override {
        return event_system.mouse_events;
    }

    EventPublisher<KeyboardEventType, KeyboardEvent>& get_keyboard_publisher() override {
        return event_system.keyboard_events;
    }

    EventPublisher<CharKeyboardEventType, CharKeyboardEvent>& get_char_keyboard_publisher() override {
        return event_system.char_keyboard_events;
    }
};