#pragma once

#include <SDL2/SDL_timer.h>
#include <SDL_events.h>
#include <SDL_keycode.h>

#include <libtcod.hpp>
#include <libtcod/console.hpp>

#include "entity/entity_manager.hpp"
#include "hardware.pb.h"
#include "hardware/hardware_manager.hpp"
#include "hardware/program_executor.hpp"
#include "map/manager.hpp"
#include "ui/event_system.hpp"
#include "ui/render.hpp"
#include "utils/thread_pool.hpp"

struct Level;

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
    virtual EventPublisher<KeyboardChordEventType, KeyboardChordEvent>&
    get_keyboard_chord_publisher() = 0;
    virtual EventPublisher<CharKeyboardEventType, CharKeyboardEvent>&
    get_char_keyboard_publisher() = 0;
};

class EditorMode : public Mode {
    EventSystem event_system = {};
    Renderer& renderer;
    LayoutBox& box;
    TextEditor editor;
    std::vector<Level> const& levels;

   public:
    EditorMode(Renderer& renderer, LayoutBox& box,
               SoftwareManager& software_manager,
               std::vector<Level> const& levels);

    bool is_editor() override { return true; }
    bool is_primary() override { return false; }

    void on_start() override { editor.open(); }
    void on_end() override { editor.close(); }

    void render() override {
        renderer.render_text_editor(box, editor, levels.size());
    }

    void update() override {}

    EventPublisher<MouseEventType, MouseEvent>& get_mouse_publisher() override {
        return event_system.mouse_events;
    }

    EventPublisher<KeyboardEventType, KeyboardEvent>& get_keyboard_publisher()
        override {
        return event_system.keyboard_events;
    }

    EventPublisher<KeyboardChordEventType, KeyboardChordEvent>&
    get_keyboard_chord_publisher() override {
        return event_system.keyboard_chord_events;
    }

    EventPublisher<CharKeyboardEventType, CharKeyboardEvent>&
    get_char_keyboard_publisher() override {
        return event_system.char_keyboard_events;
    }
};

class PrimaryMode : public Mode {
    EventSystem event_system = {};
    LayoutBox& box;
    HardwareManager hardware_manager;
    EntityManager& entity_manager;
    MapManager& map_manager;
    MapWorld& map_world;
    Renderer& renderer;
    bool& is_reload_game;
    const ThreadPool<AsyncProgramJob>& job_pool;

   public:
    PrimaryMode(LayoutBox& box, CommandMap const& command_map,
                SoftwareManager& software_manager,
                EntityManager& entity_manager, MapManager& map_manager,
                MapWorld& map_world, Renderer& renderer, bool& is_reload_game,
                const ThreadPool<AsyncProgramJob>& job_pool);

    PrimaryMode(const ant_proto::HardwareManager msg, LayoutBox& box,
                CommandMap const& command_map,
                SoftwareManager& software_manager,
                EntityManager& entity_manager, MapManager& map_manager,
                MapWorld& map_world, Renderer& renderer, bool& is_reload_game,
                const ThreadPool<AsyncProgramJob>& job_pool);

    void initialize(SoftwareManager& software_manager);
    bool is_editor() override { return false; }
    bool is_primary() override { return true; }
    void on_start() override {}
    void on_end() override {}
    void render() override;
    void update() override;

    EventPublisher<MouseEventType, MouseEvent>& get_mouse_publisher() override {
        return event_system.mouse_events;
    }

    EventPublisher<KeyboardEventType, KeyboardEvent>& get_keyboard_publisher()
        override {
        return event_system.keyboard_events;
    }

    EventPublisher<KeyboardChordEventType, KeyboardChordEvent>&
    get_keyboard_chord_publisher() override {
        return event_system.keyboard_chord_events;
    }

    EventPublisher<CharKeyboardEventType, CharKeyboardEvent>&
    get_char_keyboard_publisher() override {
        return event_system.char_keyboard_events;
    }

    ant_proto::HardwareManager get_proto() const {
        ant_proto::HardwareManager msg;
        return msg;
    }
};
