#pragma once

#include <SDL2/SDL_timer.h>
#include <SDL_events.h>
#include <SDL_keycode.h>

#include <libtcod.hpp>
#include <libtcod/console.hpp>
#include <vector>

#include "entity/entity_manager.hpp"
#include "entity/entity_data.hpp"
#include "ui/event_system.hpp"
#include "ui/render.hpp"
#include "ui/text_editor_handler.hpp"
#include "ui/ui_handlers.hpp"
#include "hardware/hardware_manager.hpp"

class Mode {
   public:
    virtual bool is_editor() = 0;
    virtual bool is_primary() = 0;
    virtual void render() = 0;
    virtual void update() = 0;
    virtual void on_start() = 0;
    virtual void on_end() = 0;
    virtual EventPublisher<MouseEventType, MouseEvent>&
    get_mouse_publisher() = 0;
    virtual EventPublisher<KeyboardEventType, KeyboardEvent>&
    get_keyboard_publisher() = 0;
    virtual EventPublisher<CharKeyboardEventType, CharKeyboardEvent>&
    get_char_keyboard_publisher() = 0;
};

class EditorMode : public Mode {
    EventSystem event_system;
    Renderer& renderer;
    LayoutBox& box;
    TextEditor editor;
    std::vector<Worker*> const& workers;

   public:
    EditorMode(Renderer& renderer, LayoutBox& box, SoftwareManager& software_manager, std::vector<Worker*> const& workers)
        : renderer(renderer), box(box), editor(software_manager), workers(workers) {
        // text editor listeners
        event_system.keyboard_events.add(RETURN_KEY_EVENT,
                                         new NewLineHandler(editor));
        event_system.keyboard_events.add(BACKSPACE_KEY_EVENT,
                                         new BackspaceHandler(editor));
        event_system.keyboard_events.add(LEFT_KEY_EVENT,
                                         new MoveCursorLeftHandler(editor));
        event_system.keyboard_events.add(RIGHT_KEY_EVENT,
                                         new MoveCursorRightHandler(editor));
        event_system.keyboard_events.add(UP_KEY_EVENT,
                                         new MoveCursorUpHandler(editor));
        event_system.keyboard_events.add(DOWN_KEY_EVENT,
                                         new MoveCursorDownHandler(editor));

        // text editor printable char listener
        event_system.char_keyboard_events.add(CHAR_KEY_EVENT,
                                              new EditorKeyHandler(editor));
    }

    bool is_editor() override { return true; }
    bool is_primary() override { return false; }

    void on_start() override { editor.open(); }
    void on_end() override { editor.close(); }

    void render() override {
        renderer.render_text_editor(box, editor, workers.size());
    }

    void update() override {}

    EventPublisher<MouseEventType, MouseEvent>& get_mouse_publisher() override {
        return event_system.mouse_events;
    }

    EventPublisher<KeyboardEventType, KeyboardEvent>& get_keyboard_publisher()
        override {
        return event_system.keyboard_events;
    }

    EventPublisher<CharKeyboardEventType, CharKeyboardEvent>&
    get_char_keyboard_publisher() override {
        return event_system.char_keyboard_events;
    }
};

class PrimaryMode : public Mode {
    EventSystem event_system;
    LayoutBox& box;
    HardwareManager hardware_manager;
    EntityManager& entity_manager;
    Renderer& renderer;
    ulong clock_timeout_1000ms;

   public:
    PrimaryMode(LayoutBox& box, CommandMap const& command_map, SoftwareManager& software_manager, EntityManager& entity_manager, Renderer& renderer)
        : box(box),
          hardware_manager(command_map),
          entity_manager(entity_manager),
          renderer(renderer),
          clock_timeout_1000ms(SDL_GetTicks64()) {
        
        initialize(software_manager);
    }
    
    PrimaryMode(Unpacker& p, LayoutBox& box, CommandMap const& command_map, SoftwareManager& software_manager, EntityManager& entity_manager, Renderer& renderer)
        : box(box),
          hardware_manager(p, command_map),
          entity_manager(entity_manager),
          renderer(renderer),
          clock_timeout_1000ms(SDL_GetTicks64()) {
        SPDLOG_DEBUG("Unpacking primary mode object"); 
        initialize(software_manager);
        entity_manager.rebuild_workers(hardware_manager, software_manager);
        SPDLOG_TRACE("Completed unpacking the primary mode object");
    }
          
    
    void initialize(SoftwareManager& software_manager) {
        SPDLOG_DEBUG("Adding the primary mode event system subscriptions");
        event_system.keyboard_events.add(
            LEFT_KEY_EVENT,
            new MoveLeftHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(
            RIGHT_KEY_EVENT,
            new MoveRightHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(
            UP_KEY_EVENT,
            new MoveUpHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(
            DOWN_KEY_EVENT,
            new MoveDownHandler(entity_manager.map, entity_manager.player));

        event_system.keyboard_events.add(
            H_KEY_EVENT,
            new MoveLeftHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(
            L_KEY_EVENT,
            new MoveRightHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(
            K_KEY_EVENT,
            new MoveUpHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(
            J_KEY_EVENT,
            new MoveDownHandler(entity_manager.map, entity_manager.player));
        event_system.keyboard_events.add(
            A_KEY_EVENT,
            new CreateAntHandler(entity_manager, hardware_manager, software_manager));

        // click listeners
        event_system.mouse_events.add(
            LEFT_MOUSE_EVENT, new ClickHandler(entity_manager.map, renderer));
        SPDLOG_TRACE("Completed adding the primary mode event system subscriptions");
    }

    bool is_editor() override { return false; }
    bool is_primary() override { return true; }

    void on_start() override {}
    void on_end() override {}

    void render() override {
        // SPDLOG_TRACE("Rendering engine");

        // draw the map
        renderer.render_map(box, entity_manager.map, entity_manager.map_window);

        // draw the buildings
        // SPDLOG_TRACE("Rendering {} buildings", buildings.size());
        for(auto building : entity_manager.buildings) {
            renderer.render_building(box, *building, entity_manager.map_window);
        }

        // draw the workers
        // SPDLOG_TRACE("Rendering {} workers", workers.size());
        for(auto ant : entity_manager.workers) {
            renderer.render_ant(box, entity_manager.map, ant->get_data(),
                                entity_manager.map_window);
        }
        renderer.render_ant(box, entity_manager.map, entity_manager.player.get_data(),
            entity_manager.map_window);
    }

    void update() override {
        entity_manager.update();

        // SPDLOG_TRACE("Checking for clock pulse");
        if(clock_timeout_1000ms < SDL_GetTicks64()) {
            // SPDLOG_TRACE("Detected clock pulse");
            for(ClockController* c : hardware_manager) {
                c->handleClockPulse();
            }
            clock_timeout_1000ms += 1000;
        }
    }

    EventPublisher<MouseEventType, MouseEvent>& get_mouse_publisher() override {
        return event_system.mouse_events;
    }

    EventPublisher<KeyboardEventType, KeyboardEvent>& get_keyboard_publisher()
        override {
        return event_system.keyboard_events;
    }

    EventPublisher<CharKeyboardEventType, CharKeyboardEvent>&
    get_char_keyboard_publisher() override {
        return event_system.char_keyboard_events;
    }

    friend Packer& operator<<(Packer& p, PrimaryMode const& obj) {
        SPDLOG_DEBUG("Packing the primary mode object");
        return p << obj.hardware_manager;
    }
};