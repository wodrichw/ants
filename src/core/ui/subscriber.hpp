#pragma once

#include <unordered_set>
#include <spdlog/spdlog.h>

using ulong = unsigned long;

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
    COLON_KEY_EVENT, SPACE_KEY_EVENT, BACK_SLASH_KEY_EVENT, UNKNOWN_KEY_EVENT
};
using KeyboardChordEventType = std::pair<KeyboardEventType,KeyboardEventType>;

enum CharKeyboardEventType
{
    CHAR_KEY_EVENT
};

struct MouseEvent
{
    long x = 0, y = 0;
    MouseEventType type = UNKNOWN_MOUSE_EVENT;
};

struct KeyboardEvent
{
    KeyboardEventType type = UNKNOWN_KEY_EVENT;
};

struct KeyboardChordEvent
{
    KeyboardChordEventType chord = std::make_pair(UNKNOWN_KEY_EVENT, UNKNOWN_KEY_EVENT);
    void set_key(KeyboardEventType key) {
        if (chord.first == UNKNOWN_KEY_EVENT) {
            SPDLOG_DEBUG("Set first chord key: {}", static_cast<ulong>(key));
            chord.first = key;
            return;
        }

        SPDLOG_DEBUG("Set second chord key: {}", static_cast<ulong>(key));
        chord.second = key;
    }

    void unset_key(KeyboardEventType key) {
        if (chord.first == key) {
            SPDLOG_DEBUG("Unset first chord key: {}", static_cast<ulong>(key));
            chord.first = chord.second;
            chord.second = UNKNOWN_KEY_EVENT;
            return;
        }

        if (chord.second == key) {
            SPDLOG_DEBUG("Unset second chord key: {}", static_cast<ulong>(key));
            chord.second = UNKNOWN_KEY_EVENT;
        }
    }
};

struct CharKeyboardEvent
{
    CharKeyboardEventType type = CHAR_KEY_EVENT;
    char key = 0;
};

namespace std {
    template <>
    struct hash<MouseEvent> {
        std::size_t operator()(MouseEvent const& e) const noexcept {
            return static_cast<std::size_t>(e.type);
        }
    };
    
    template <>
    struct hash<KeyboardEvent> {
        std::size_t operator()(KeyboardEvent const& e) const noexcept {
            return static_cast<std::size_t>(e.type);
        }
    };

    template <>
    struct hash<CharKeyboardEvent> {
        std::size_t operator()(CharKeyboardEvent const& e) const noexcept {
            return static_cast<std::size_t>(e.type);
        }
    };

    template <>
    struct hash<KeyboardChordEventType> {
        std::size_t operator()(KeyboardChordEventType const& e) const noexcept {
            ulong constexpr HASH_MULT = 196613UL;
            return static_cast<std::size_t>(e.first) * HASH_MULT + static_cast<std::size_t>(e.second);
        }
    };

    template <>
    struct hash<KeyboardChordEvent> {
        std::size_t operator()(KeyboardChordEvent const& e) const noexcept {
            std::hash<KeyboardChordEventType> hash;
            return hash(e.chord);
        }
    };
}

template <typename Event>
struct Subscriber
{
    virtual void operator()(Event const& event) = 0;
};
