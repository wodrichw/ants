#pragma once

#include <SDL2/SDL_timer.h>
#include <SDL_events.h>
#include <SDL_keycode.h>

#include <libtcod.hpp>
#include <libtcod/console.hpp>

#include "entity/entity_manager.hpp"
#include "entity/entity_data.hpp"
#include "hardware/program_executor.hpp"
#include "ui/event_system.hpp"
#include "ui/render.hpp"
#include "ui/text_editor_handler.hpp"
#include "ui/ui_handlers.hpp"
#include "hardware/hardware_manager.hpp"
#include "utils/thread_pool.hpp"

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

    EventPublisher<KeyboardChordEventType, KeyboardChordEvent>& get_keyboard_chord_publisher()
        override {
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
    Renderer& renderer;
    bool& is_reload_game;
    const ThreadPool<AsyncProgramJob>& job_pool;

   public:
    PrimaryMode(
            LayoutBox& box,
            CommandMap const& command_map,
            SoftwareManager& software_manager,
            EntityManager& entity_manager,
            Renderer& renderer,
            bool& is_reload_game,
            const ThreadPool<AsyncProgramJob>& job_pool
    ): 
            box(box),
            hardware_manager(command_map),
            entity_manager(entity_manager),
            renderer(renderer),
            is_reload_game(is_reload_game),
            job_pool(job_pool)
    {

        initialize(software_manager);
    }

    PrimaryMode(
            Unpacker& p,
            LayoutBox& box,
            CommandMap const& command_map,
            SoftwareManager& software_manager,
            EntityManager& entity_manager,
            Renderer& renderer,
            bool& is_reload_game,
            const ThreadPool<AsyncProgramJob>& job_pool
     ):
            box(box),
            hardware_manager(p, command_map),
            entity_manager(entity_manager),
            renderer(renderer),
            is_reload_game(is_reload_game),
            job_pool(job_pool)
    {
        SPDLOG_DEBUG("Unpacking primary mode object");
        initialize(software_manager);
        entity_manager.rebuild_workers(hardware_manager, software_manager);
        SPDLOG_TRACE("Completed unpacking the primary mode object");
    }


    void initialize(SoftwareManager& software_manager) {
        SPDLOG_DEBUG("Adding the primary mode event system subscriptions");
        event_system.keyboard_events.add(
            LEFT_KEY_EVENT,
            new MoveHandler(entity_manager.map, entity_manager.player, -1, 0));
        event_system.keyboard_events.add(
            RIGHT_KEY_EVENT,
            new MoveHandler(entity_manager.map, entity_manager.player, 1, 0));
        event_system.keyboard_events.add(
            UP_KEY_EVENT,
            new MoveHandler(entity_manager.map, entity_manager.player, 0, -1));
        event_system.keyboard_events.add(
            DOWN_KEY_EVENT,
            new MoveHandler(entity_manager.map, entity_manager.player, 0, 1));

        event_system.keyboard_events.add(
            H_KEY_EVENT,
            new MoveHandler(entity_manager.map, entity_manager.player, -1, 0));
        event_system.keyboard_events.add(
            L_KEY_EVENT,
            new MoveHandler(entity_manager.map, entity_manager.player, 1, 0));
        event_system.keyboard_events.add(
            K_KEY_EVENT,
            new MoveHandler(entity_manager.map, entity_manager.player, 0, -1));
        event_system.keyboard_events.add(
            J_KEY_EVENT,
            new MoveHandler(entity_manager.map, entity_manager.player, 0, 1));
        event_system.keyboard_events.add(
            A_KEY_EVENT,
            new CreateAntHandler(entity_manager, hardware_manager, software_manager));

        event_system.keyboard_chord_events.add(
            {D_KEY_EVENT, LEFT_KEY_EVENT},
            new DigHandler(entity_manager.map, entity_manager.player, entity_manager.player.inventory, -1, 0)
        );
        event_system.keyboard_chord_events.add(
            {D_KEY_EVENT, RIGHT_KEY_EVENT},
            new DigHandler(entity_manager.map, entity_manager.player, entity_manager.player.inventory, 1, 0)
        );
        event_system.keyboard_chord_events.add(
            {D_KEY_EVENT, UP_KEY_EVENT},
            new DigHandler(entity_manager.map, entity_manager.player, entity_manager.player.inventory, 0, -1)
        );
        event_system.keyboard_chord_events.add(
            {D_KEY_EVENT, DOWN_KEY_EVENT},
            new DigHandler(entity_manager.map, entity_manager.player, entity_manager.player.inventory, 0, 1)
        );
        event_system.keyboard_chord_events.add(
            {D_KEY_EVENT, H_KEY_EVENT},
            new DigHandler(entity_manager.map, entity_manager.player, entity_manager.player.inventory, -1, 0)
        );
        event_system.keyboard_chord_events.add(
            {D_KEY_EVENT, L_KEY_EVENT},
            new DigHandler(entity_manager.map, entity_manager.player, entity_manager.player.inventory, 1, 0)
        );
        event_system.keyboard_chord_events.add(
            {D_KEY_EVENT, K_KEY_EVENT},
            new DigHandler(entity_manager.map, entity_manager.player, entity_manager.player.inventory, 0, -1)
        );
        event_system.keyboard_chord_events.add(
            {D_KEY_EVENT, J_KEY_EVENT},
            new DigHandler(entity_manager.map, entity_manager.player, entity_manager.player.inventory, 0, 1)
        );

        event_system.keyboard_events.add(
            R_KEY_EVENT,
            new ReloadGameHandler(is_reload_game)
        );

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

    //TODO: (job_pool) make sure that we don't call handleClockPulse until all the jobs on the job_pool
    // is finished
    void update() override {
        entity_manager.update();

        for(ProgramExecutor* exec : hardware_manager) {
            exec->reset();
        }

        for(ProgramExecutor* exec : hardware_manager) {
            exec->execute_async();
        }

        job_pool.await_jobs();

        for(ProgramExecutor* exec : hardware_manager) {
            exec->execute_sync();
        }
    }

    EventPublisher<MouseEventType, MouseEvent>& get_mouse_publisher() override {
        return event_system.mouse_events;
    }

    EventPublisher<KeyboardEventType, KeyboardEvent>& get_keyboard_publisher()
        override {
        return event_system.keyboard_events;
    }

    EventPublisher<KeyboardChordEventType, KeyboardChordEvent>& get_keyboard_chord_publisher()
        override {
        return event_system.keyboard_chord_events;
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
