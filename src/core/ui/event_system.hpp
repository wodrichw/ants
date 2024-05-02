#include <unordered_map>
#include <vector>
#include <algorithm>

#include <SDL2/SDL_events.h>

#include "spdlog/spdlog.h"
#include "subscriber.hpp"

enum MouseEventType
{
    LEFT_MOUSE_EVENT,
    RIGHT_MOUSE_EVENT,
    MIDDLE_MOUSE_EVENT,
    UNKNOWN_MOUSE_EVENT
};
enum KeyboardEventType
{
    SLASH_KEY_EVENT, A_KEY_EVENT, B_KEY_EVENT, C_KEY_EVENT, D_KEY_EVENT,
    E_KEY_EVENT, F_KEY_EVENT,  G_KEY_EVENT, H_KEY_EVENT, I_KEY_EVENT,
    J_KEY_EVENT, K_KEY_EVENT, L_KEY_EVENT, M_KEY_EVENT, N_KEY_EVENT,
    O_KEY_EVENT, P_KEY_EVENT, Q_KEY_EVENT, R_KEY_EVENT, S_KEY_EVENT,
    T_KEY_EVENT, U_KEY_EVENT, V_KEY_EVENT, W_KEY_EVENT, X_KEY_EVENT,
    Y_KEY_EVENT, Z_KEY_EVENT, LEFT_KEY_EVENT, RIGHT_KEY_EVENT,
    UP_KEY_EVENT, DOWN_KEY_EVENT, RETURN_KEY_EVENT, BACKSPACE_KEY_EVENT,
    SPACE_KEY_EVENT, UNKNOWN_KEY_EVENT
};

enum CharKeyboardEventType
{
    CHAR_KEY_EVENT
};

struct MouseEvent
{
    long x, y;
    MouseEventType type;
    MouseEvent(long x, long y, MouseEventType type) : x(x), y(y), type(type) {}
};

struct KeyboardEvent
{
    KeyboardEventType type;
    KeyboardEvent(KeyboardEventType type) : type(type) {}
};

struct CharKeyBoardEvent
{
    CharKeyboardEventType type = CHAR_KEY_EVENT;
    char key;
    CharKeyBoardEvent(char key) : key(key) {}
};

template <typename Enum, typename Event>
struct EventPublisher
{
    void add(Enum key, Subscriber<Event>* subscriber) {
        get(key)->push_back(subscriber);
    }

    void remove(Enum key, Subscriber<Event>* subscriber) {
        auto it = subscriber_map.find(key);
        if (it != subscriber_map.end()) {
            return;
        }

        auto& subscribers = *it->second;
        subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), subscriber), subscribers.end());
    }

    void notify(Event const& event) {
        auto it = subscriber_map.find(event.type);
        if (it != subscriber_map.end()) {
            return;
        }
        for (auto& subscriber : *it->second) {
            subscriber->update(event);
        }
    }
    
    ~EventPublisher() {
        for (auto& pair : subscriber_map) {
            delete pair.second;
        }
    }

private:

    std::vector<Subscriber<Event>*>* get(Enum key) {
        auto it = subscriber_map.find(key);
        if (it != subscriber_map.end()) {
            return (*it).second;
        }
        return subscriber_map[key] = new std::vector<Subscriber<Event>*>();
    }

    std::unordered_map<Enum, std::vector<Subscriber<Event>*>*> subscriber_map;
};


struct EventSystem {
    EventPublisher<MouseEventType, MouseEvent> mouse_events;
    EventPublisher<KeyboardEventType, KeyboardEvent> keyboard_events;
    EventPublisher<CharKeyboardEventType, CharKeyBoardEvent> char_keyboard_events;

    void update() {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    handle_quit_event();
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    handle_mouse_event(event.button);
                    break;

                case SDL_KEYDOWN:
                    handle_keyboard_event(event.key.keysym);
                    break;
            }
        }
    }

private:
    void handle_quit_event() {
        SPDLOG_INFO("Detected SDL_QUIT event, exiting");
        std::exit(EXIT_SUCCESS);
    }

    void handle_mouse_event(SDL_MouseButtonEvent const& event) {
        MouseEventType type = get_mouse_type(event.button);
        mouse_events.notify(MouseEvent(event.x, event.y, type));
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

    void handle_keyboard_event(SDL_Keysym const& event) {
        KeyboardEventType type = get_keyboard_type(event.sym);
        keyboard_events.notify(KeyboardEvent(type));

        if (type == UNKNOWN_KEY_EVENT) {
            return;
        }

        SDL_Keycode key = get_keyboard_key(event);
        char_keyboard_events.notify(CharKeyBoardEvent(key));
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
            default: return UNKNOWN_KEY_EVENT;
        }
    }
    
    SDL_Keycode get_keyboard_key(SDL_Keysym const& event) {
        if(!(event.mod & (KMOD_RSHIFT | KMOD_LSHIFT))) return event.sym;
        if(event.sym == SDLK_3) return SDLK_HASH;
        if(event.sym == SDLK_SEMICOLON) return SDLK_COLON;
        return event.sym;
    }
};