#pragma once

#include "SAJ_lib.h"



enum KeyCodeBinding
{
    KEY_MOUSE_LEFT,
    KEY_MOUSE_RIGHT,
    KEY_MOUSE_MIDDLE,

    KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F,KEY_G,KEY_H,KEY_I,
    KEY_J,KEY_K,KEY_L,KEY_M,KEY_N,KEY_O,KEY_P,KEY_Q,
    KEY_R,KEY_S,KEY_T,KEY_U,KEY_V,KEY_W,KEY_X,KEY_Y,KEY_Z,

    KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,

    KEY_SPACE,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_ALT,
    KEY_TAB,
    KEY_ENTER,
    /*
    TODO: ADD More keys
    */

    KEY_ARROW_LEFT,KEY_ARROW_RIGHT,KEY_ARROW_UP,KEY_ARROW_DOWN,

    KEYCODE_COUNT = 255
};

struct KeyState
{
    b8 isDown;
    b8 isPressed;
    b8 isReleased;
    unsigned char halfTransitionCount;

};
struct Input
{
    //screen
    IVec2 ScreenSize;
    IVec2 mousePosition;
    IVec2 relativeMousePosition;

    //World
    IVec2 prevMousePosition;
    IVec2 mousePositionWorld;
    IVec2 relativeMousePositionWorld;

    KeyState keys[KEYCODE_COUNT];

};

static Input* input;

bool key_in_frame_pressed(KeyCodeBinding keyCode)
{
    KeyState key = input->keys[keyCode];
    //is being pressed or have been pressed 1+ times
    return key.isDown || key.halfTransitionCount >= 1;
}
bool key_in_frame_released(KeyCodeBinding keyCode)
{
    KeyState key = input->keys[keyCode];
    //is being pressed or have been pressed 1+ times
    return !key.isDown || key.halfTransitionCount >= 1;
}

bool key_is_down(KeyCodeBinding keyCode)
{
    return input->keys[keyCode].isDown;
}