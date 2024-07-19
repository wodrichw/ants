#include "app/game_state.hpp"
#include "app/game_mode.hpp"

GameState::GameState(Mode* primary_mode, Mode* editor_mode): mode(primary_mode),
        primary_mode(primary_mode), editor_mode(editor_mode) {
            mode->on_start();
    }

bool GameState::is_editor() { return mode->is_editor(); }
bool GameState::is_primary() { return mode->is_primary(); }

void GameState::render() { mode->render(); }
void GameState::update() { mode->update(); }

void GameState::toggle_editor() {
    mode->on_end();
    if (mode->is_editor()) {
        mode = primary_mode;
    } else {
        mode = editor_mode;
    }
    mode->on_start();
}

 EventPublisher<MouseEventType, MouseEvent>& GameState::get_mouse_publisher() {
    return mode->get_mouse_publisher();
}

EventPublisher<KeyboardEventType, KeyboardEvent>& GameState::get_keyboard_publisher() {
    return mode->get_keyboard_publisher();
}

EventPublisher<KeyboardChordEventType, KeyboardChordEvent>& GameState::get_keyboard_chord_publisher() {
    return mode->get_keyboard_chord_publisher();
}

EventPublisher<CharKeyboardEventType, CharKeyboardEvent>& GameState::get_char_keyboard_publisher() {
    return mode->get_char_keyboard_publisher();
}
