#pragma once
#include <ui/event_system.hpp>

class Mode;

class GameState {
    public:
    Mode* mode;
    Mode* primary_mode;
    Mode* editor_mode;

    GameState(Mode* primary_mode, Mode* editor_mode);

    bool is_editor();
    bool is_primary();

    void render();
    void update();
    void toggle_editor();

    EventPublisher<MouseEventType, MouseEvent>& get_mouse_publisher();
    EventPublisher<KeyboardEventType, KeyboardEvent>& get_keyboard_publisher();
    EventPublisher<CharKeyboardEventType, CharKeyboardEvent>& get_char_keyboard_publisher();
};
