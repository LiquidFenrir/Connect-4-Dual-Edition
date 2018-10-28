#pragma once

#include "common.h"

enum JoystickKeys {
    JKEY_A,
    JKEY_B,
    JKEY_X,
    JKEY_Y,

    JKEY_LSTICK,
    JKEY_RSTICK,

    JKEY_L,
    JKEY_R,

    JKEY_ZL,
    JKEY_ZR,

    JKEY_PLUS,
    JKEY_MINUS,

    JKEY_DLEFT,
    JKEY_DUP,
    JKEY_DRIGHT,
    JKEY_DDOWN,

    JOYSTICK_KEYS_AMOUNT
};

class Input {
    public:
        Input();
        ~Input();

        void get();
        
        std::array<bool, JOYSTICK_KEYS_AMOUNT> keys;
};
