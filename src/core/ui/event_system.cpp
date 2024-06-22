#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_timer.h>

#include "event_system.hpp"
#include "spdlog/spdlog.h"

void handle_quit_event() {
    SPDLOG_INFO("Detected SDL_QUIT event, exiting");
    std::exit(EXIT_SUCCESS);
}

MouseEventType get_mouse_type(char button) {
    switch (button) {
        case SDL_BUTTON_LEFT:
            return LEFT_MOUSE_EVENT;
        case SDL_BUTTON_RIGHT:
            return RIGHT_MOUSE_EVENT;
        case SDL_BUTTON_MIDDLE:
            return MIDDLE_MOUSE_EVENT;
        default:
            return UNKNOWN_MOUSE_EVENT;
    }
}

void set_mouse_type(SDL_MouseButtonEvent const& event, MouseEvent& mouse_event) {
    SPDLOG_DEBUG("Getting mouse event: x={}, y={}, button={}", event.x, event.y, event.button);
    mouse_event.x = event.x;
    mouse_event.y = event.y;
    mouse_event.type = get_mouse_type(event.button);
}

KeyboardEventType get_keyboard_type(SDL_Keycode event) {
    switch (event) {
        case SDLK_SLASH: return SLASH_KEY_EVENT;
        case SDLK_a: return A_KEY_EVENT;
        case SDLK_b: return B_KEY_EVENT;
        case SDLK_c: return C_KEY_EVENT;
        case SDLK_d: return D_KEY_EVENT;
        case SDLK_e: return E_KEY_EVENT;
        case SDLK_f: return F_KEY_EVENT;
        case SDLK_g: return G_KEY_EVENT;
        case SDLK_h: return H_KEY_EVENT;
        case SDLK_i: return I_KEY_EVENT;
        case SDLK_j: return J_KEY_EVENT;
        case SDLK_k: return K_KEY_EVENT;
        case SDLK_l: return L_KEY_EVENT;
        case SDLK_m: return M_KEY_EVENT;
        case SDLK_n: return N_KEY_EVENT;
        case SDLK_o: return O_KEY_EVENT;
        case SDLK_p: return P_KEY_EVENT;
        case SDLK_q: return Q_KEY_EVENT;
        case SDLK_r: return R_KEY_EVENT;
        case SDLK_s: return S_KEY_EVENT;
        case SDLK_t: return T_KEY_EVENT;
        case SDLK_u: return U_KEY_EVENT;
        case SDLK_v: return V_KEY_EVENT;
        case SDLK_w: return W_KEY_EVENT;
        case SDLK_x: return X_KEY_EVENT;
        case SDLK_y: return Y_KEY_EVENT;
        case SDLK_z: return Z_KEY_EVENT;
        case SDLK_LEFT: return LEFT_KEY_EVENT;
        case SDLK_RIGHT: return RIGHT_KEY_EVENT;
        case SDLK_UP: return UP_KEY_EVENT;
        case SDLK_DOWN: return DOWN_KEY_EVENT;
        case SDLK_RETURN: return RETURN_KEY_EVENT;
        case SDLK_BACKSPACE: return BACKSPACE_KEY_EVENT;
        case SDLK_SPACE: return SPACE_KEY_EVENT;
        case SDLK_COLON: return COLON_KEY_EVENT;
        case SDLK_BACKSLASH: return BACK_SLASH_KEY_EVENT;
        default: return UNKNOWN_KEY_EVENT;
    }
}

void set_keyboard_type(SDL_Keysym const& event, KeyboardEvent& keyboard_event) {
    SPDLOG_DEBUG("Setting keyboard event: key={}, mod={}", event.sym, event.mod);
    keyboard_event.type = get_keyboard_type(event.sym);
}

void set_keyboard_chord_type(SDL_Keysym const& event, KeyboardChordEvent& keyboard_event) {
    SPDLOG_DEBUG("Setting keyboard event: key={}, mod={}", event.sym, event.mod);
    keyboard_event.set_key(get_keyboard_type(event.sym));
}

void unset_keyboard_chord_type(SDL_Keysym const& event, KeyboardChordEvent& keyboard_event) {
    SPDLOG_DEBUG("Unsetting keyboard event: key={}, mod={}", event.sym, event.mod);
    keyboard_event.unset_key(get_keyboard_type(event.sym));
}

SDL_Keycode get_keyboard_key(SDL_Keysym const& event) {
    if(!(event.mod & (KMOD_RSHIFT | KMOD_LSHIFT))) return event.sym;
    if(event.sym == SDLK_3) return SDLK_HASH;
    if(event.sym == SDLK_SEMICOLON) return SDLK_COLON;
    return event.sym;
}

void set_char_keyboard_type(SDL_Keysym const& event, CharKeyboardEvent& char_keyboard_event) {
    char_keyboard_event.key = get_keyboard_key(event);
    SPDLOG_DEBUG("Getting char keyboard event: key={}, mod={}", char_keyboard_event.key, event.mod);
}