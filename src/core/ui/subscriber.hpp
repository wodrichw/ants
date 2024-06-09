#pragma once

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

struct CharKeyboardEvent
{
    CharKeyboardEventType type = CHAR_KEY_EVENT;
    char key = 0;
};

template <typename Event>
struct Subscriber
{
    virtual void operator()(Event const& event) = 0;
};

