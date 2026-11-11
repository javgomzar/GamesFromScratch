#pragma once

#include "platform.h"
#include "mathematics.h"


struct Button_State {
    bool is_down;
    bool was_down;
    bool just_pressed;
    bool just_lifted;
};

inline const int NUMBER_OF_CONTROLLER_BUTTONS = 16;

struct Controller_Input {
    bool any;
    V2 left_joystick;
    V2 right_joystick;
    union {
        Button_State buttons[NUMBER_OF_CONTROLLER_BUTTONS];
        struct {
            Button_State A_button;
            Button_State B_button;
            Button_State X_button;
            Button_State Y_button;
            Button_State pad_up;
            Button_State pad_down;
            Button_State pad_left;
            Button_State pad_right;
            Button_State RB;
            Button_State LB;
            Button_State RS;
            Button_State LS;
            Button_State RT;
            Button_State LT;
            Button_State start;
            Button_State back;
        };
    };
};

inline const int NUMBER_OF_KEYS = 57;

struct Keyboard_Input {
    union {
        Button_State keys[NUMBER_OF_KEYS];
        struct {
            Button_State one;
            Button_State two;
            Button_State three;
            Button_State four;
            Button_State five;
            Button_State six;
            Button_State seven;
            Button_State eight;
            Button_State nine;
            Button_State zero;
            Button_State Q;
            Button_State W;
            Button_State E;
            Button_State R;
            Button_State T;
            Button_State Y;
            Button_State U;
            Button_State I;
            Button_State O;
            Button_State P;
            Button_State A;
            Button_State S;
            Button_State D;
            Button_State F;
            Button_State G;
            Button_State H;
            Button_State J;
            Button_State K;
            Button_State L;
            Button_State Z;
            Button_State X;
            Button_State C;
            Button_State V;
            Button_State B;
            Button_State N;
            Button_State M;
            Button_State up;
            Button_State down;
            Button_State left;
            Button_State right;
            Button_State escape;
            Button_State space;
            Button_State enter;
            Button_State shift;
            Button_State control;
            Button_State alt;
            Button_State F1;
            Button_State F2;
            Button_State F3;
            Button_State F4;
            Button_State F5;
            Button_State F6;
            Button_State F7;
            Button_State F8;
            Button_State F9;
            Button_State F10;
            Button_State F11;
            Button_State F12;
            Button_State page_up;
            Button_State page_down;
        };
    };
    bool any;
};

struct Mouse_Input {
    Button_State left_click;
    Button_State middle_click;
    Button_State right_click;
    V2 cursor;
    V2 last_cursor;
    int16 wheel;
};

enum Input_Mode {
    Keyboard,
    Controller
};

struct Input {
    Input_Mode mode;
    Controller_Input controller;
    Keyboard_Input keyboard;
    Mouse_Input mouse;
};

void UpdatePreviousInput(Input* input);
void PressButton(Button_State* button);
void LiftButton(Button_State* button);
Button_State* GetKey(Input* input, char key);
void PressKey(Input* input, char key);
void LiftKey(Input* input, char key);
