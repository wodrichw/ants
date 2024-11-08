#include "app/game_mode.hpp"
#include "ui/text_editor_handler.hpp"
#include "ui/ui_handlers.hpp"

EditorMode::EditorMode(Renderer& renderer, LayoutBox& box, SoftwareManager& software_manager, std::vector<Level> const& levels)
: renderer(renderer), box(box), editor(software_manager), levels(levels) {
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

PrimaryMode::PrimaryMode(
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

PrimaryMode::PrimaryMode(
        const ant_proto::HardwareManager msg,
        LayoutBox& box,
        CommandMap const& command_map,
        SoftwareManager& software_manager,
        EntityManager& entity_manager,
        Renderer& renderer,
        bool& is_reload_game,
        const ThreadPool<AsyncProgramJob>& job_pool
 ):
        box(box),
        hardware_manager(msg, command_map),
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

void PrimaryMode::initialize(SoftwareManager& software_manager) {
    SPDLOG_DEBUG("Adding the primary mode event system subscriptions");

    // MOVE PLAYER EVENTS
    event_system.keyboard_events.add(
        LEFT_KEY_EVENT,
        new MoveHandler(entity_manager.map_manager, entity_manager.player, entity_manager.current_depth, entity_manager.player_depth, true, -1, 0));
    event_system.keyboard_events.add(
        RIGHT_KEY_EVENT,
        new MoveHandler(entity_manager.map_manager, entity_manager.player,entity_manager.current_depth, entity_manager.player_depth, true,  1, 0));
    event_system.keyboard_events.add(
        UP_KEY_EVENT,
        new MoveHandler(entity_manager.map_manager, entity_manager.player, entity_manager.current_depth, entity_manager.player_depth, true, 0, -1));
    event_system.keyboard_events.add(
        DOWN_KEY_EVENT,
        new MoveHandler(entity_manager.map_manager, entity_manager.player, entity_manager.current_depth, entity_manager.player_depth, true, 0, 1));
    event_system.keyboard_events.add(
        H_KEY_EVENT,
        new MoveHandler(entity_manager.map_manager, entity_manager.player, entity_manager.current_depth, entity_manager.player_depth, true, -1, 0));
    event_system.keyboard_events.add(
        L_KEY_EVENT,
        new MoveHandler(entity_manager.map_manager, entity_manager.player, entity_manager.current_depth, entity_manager.player_depth, true, 1, 0));
    event_system.keyboard_events.add(
        K_KEY_EVENT,
        new MoveHandler(entity_manager.map_manager, entity_manager.player, entity_manager.current_depth, entity_manager.player_depth, true, 0, -1));
    event_system.keyboard_events.add(
        J_KEY_EVENT,
        new MoveHandler(entity_manager.map_manager, entity_manager.player, entity_manager.current_depth, entity_manager.player_depth, true, 0, 1));

    // ADD ANT EVENT
    event_system.keyboard_events.add(
        A_KEY_EVENT,
        new CreateAntHandler(entity_manager, hardware_manager, software_manager));
    

    // GO UP AND DOWN LEVELS EVNETS
    event_system.keyboard_events.add(
        E_KEY_EVENT,
        new ChangeLevelHandler(entity_manager, ChangeLevelHandler::Direction::UP));
    event_system.keyboard_events.add(
        Q_KEY_EVENT,
        new ChangeLevelHandler(entity_manager, ChangeLevelHandler::Direction::DOWN));


    // DIG EVENTS
    event_system.keyboard_chord_events.add(
        {D_KEY_EVENT, LEFT_KEY_EVENT},
        new DigHandler(entity_manager.map_manager, entity_manager.player, entity_manager.player.inventory, -1, 0)
    );
    event_system.keyboard_chord_events.add(
        {D_KEY_EVENT, RIGHT_KEY_EVENT},
        new DigHandler(entity_manager.map_manager, entity_manager.player, entity_manager.player.inventory, 1, 0)
    );
    event_system.keyboard_chord_events.add(
        {D_KEY_EVENT, UP_KEY_EVENT},
        new DigHandler(entity_manager.map_manager, entity_manager.player, entity_manager.player.inventory, 0, -1)
    );
    event_system.keyboard_chord_events.add(
        {D_KEY_EVENT, DOWN_KEY_EVENT},
        new DigHandler(entity_manager.map_manager, entity_manager.player, entity_manager.player.inventory, 0, 1)
    );
    event_system.keyboard_chord_events.add(
        {D_KEY_EVENT, H_KEY_EVENT},
        new DigHandler(entity_manager.map_manager, entity_manager.player, entity_manager.player.inventory, -1, 0)
    );
    event_system.keyboard_chord_events.add(
        {D_KEY_EVENT, L_KEY_EVENT},
        new DigHandler(entity_manager.map_manager, entity_manager.player, entity_manager.player.inventory, 1, 0)
    );
    event_system.keyboard_chord_events.add(
        {D_KEY_EVENT, K_KEY_EVENT},
        new DigHandler(entity_manager.map_manager, entity_manager.player, entity_manager.player.inventory, 0, -1)
    );
    event_system.keyboard_chord_events.add(
        {D_KEY_EVENT, J_KEY_EVENT},
        new DigHandler(entity_manager.map_manager, entity_manager.player, entity_manager.player.inventory, 0, 1)
    );


    // RELOAD GAME EVENT
    event_system.keyboard_events.add(
        R_KEY_EVENT,
        new ReloadGameHandler(is_reload_game));


    event_system.keyboard_events.add(ZERO_KEY_EVENT, new DefaultMapTileRendererHandler(renderer));

    // Available scents A-H (8 total)
    ScentMapTileRendererHandler* scent_map_handler = new ScentMapTileRendererHandler(renderer);
    event_system.keyboard_events.add(ONE_KEY_EVENT, scent_map_handler);
    event_system.keyboard_events.add(TWO_KEY_EVENT, scent_map_handler);
    event_system.keyboard_events.add(THREE_KEY_EVENT, scent_map_handler);
    event_system.keyboard_events.add(FOUR_KEY_EVENT, scent_map_handler);
    event_system.keyboard_events.add(FIVE_KEY_EVENT, scent_map_handler);
    event_system.keyboard_events.add(SIX_KEY_EVENT, scent_map_handler);
    event_system.keyboard_events.add(SEVEN_KEY_EVENT, scent_map_handler);
    event_system.keyboard_events.add(EIGHT_KEY_EVENT, scent_map_handler);

    // click listeners
    event_system.mouse_events.add(
        LEFT_MOUSE_EVENT, new ClickHandler(entity_manager.map_manager, renderer));
    SPDLOG_TRACE("Completed adding the prim(gdbary mode event system subscriptions");
}

void PrimaryMode::render() {
    // SPDLOG_TRACE("Rendering engine");

    // draw the map
    renderer.render_map(box, entity_manager.map_manager.get_map(), entity_manager.map_window);

    // draw the buildings
    // SPDLOG_TRACE("Rendering {} buildings", buildings.size());
    for(auto building : entity_manager.get_current_level_buildings()) {
        renderer.render_building(box, *building, entity_manager.map_window);
    }

    // draw the workers
    // SPDLOG_TRACE("Rendering {} workers", workers.size());
    for(auto ant : entity_manager.get_current_level_worker_ants()) {
        renderer.render_ant(box, entity_manager.map_manager.get_map(), ant->get_data(),
                            entity_manager.map_window);
    }

    // draw the player
    if (entity_manager.player_depth == entity_manager.current_depth) {
        renderer.render_ant(box, entity_manager.map_manager.get_map(), entity_manager.player.get_data(),
            entity_manager.map_window);
    }
}

void PrimaryMode::update() {
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
