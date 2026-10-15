#ifndef GAME_INPUT
#define GAME_INPUT

#include "GamePlatform.h"
#include "GameMath.h"


struct Button_State {
    bool is_down;
    bool was_down;
    bool just_pressed;
    bool just_lifted;
};

const int NUMBER_OF_CONTROLLER_BUTTONS = 16;

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

const int NUMBER_OF_KEYS = 57;

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
    bool Any;
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

void UpdatePreviousInput(Input* input) {
// Mouse
    input->mouse.left_click.was_down = input->mouse.left_click.is_down;
    input->mouse.left_click.just_pressed = false;
    input->mouse.left_click.just_lifted = false;
    input->mouse.middle_click.was_down = input->mouse.middle_click.is_down;
    input->mouse.middle_click.just_pressed = false;
    input->mouse.middle_click.just_lifted = false;
    input->mouse.right_click.was_down = input->mouse.right_click.is_down;
    input->mouse.right_click.just_pressed = false;
    input->mouse.right_click.just_lifted = false;
    input->mouse.wheel = 0;

// Keyboard
    for (int i = 0; i < NUMBER_OF_KEYS; i++) {
        Button_State* key = &input->keyboard.keys[i];
        key->was_down = key->is_down;
        key->just_pressed = false;
        key->just_lifted = false;
    }

// Controller
    for (int i = 0; i < 16; i++) {
        Button_State* button = &input->controller.buttons[i];
        button->was_down = button->is_down;
        button->just_pressed = false;
        button->just_lifted = false;
    }
}

void PressButton(Button_State* button) {
    button->is_down = true;
    button->just_pressed = !button->was_down;
}

void LiftButton(Button_State* button) {
    button->is_down = false;
    button->just_lifted = button->was_down;
}

Button_State* GetKey(Input* input, char key) {
    switch(key) {
        case '0': { return &input->keyboard.zero; } break;
        case '1': { return &input->keyboard.one; } break;
        case '2': { return &input->keyboard.two; } break;
        case '3': { return &input->keyboard.three; } break;
        case '4': { return &input->keyboard.four; } break;
        case '5': { return &input->keyboard.five; } break;
        case '6': { return &input->keyboard.six; } break;
        case '7': { return &input->keyboard.seven; } break;
        case '8': { return &input->keyboard.eight; } break;
        case '9': { return &input->keyboard.nine; } break;
        case 'A': { return &input->keyboard.A; } break;
        case 'B': { return &input->keyboard.B; } break;
        case 'C': { return &input->keyboard.C; } break;
        case 'D': { return &input->keyboard.D; } break;
        case 'E': { return &input->keyboard.E; } break;
        case 'F': { return &input->keyboard.F; } break;
        case 'G': { return &input->keyboard.G; } break;
        case 'H': { return &input->keyboard.H; } break;
        case 'I': { return &input->keyboard.I; } break;
        case 'J': { return &input->keyboard.J; } break;
        case 'K': { return &input->keyboard.K; } break;
        case 'L': { return &input->keyboard.L; } break;
        case 'M': { return &input->keyboard.M; } break;
        case 'N': { return &input->keyboard.N; } break;
        case 'O': { return &input->keyboard.O; } break;
        case 'P': { return &input->keyboard.P; } break;
        case 'Q': { return &input->keyboard.Q; } break;
        case 'R': { return &input->keyboard.R; } break;
        case 'S': { return &input->keyboard.S; } break;
        case 'T': { return &input->keyboard.T; } break;
        case 'U': { return &input->keyboard.U; } break;
        case 'V': { return &input->keyboard.V; } break;
        case 'W': { return &input->keyboard.W; } break;
        case 'X': { return &input->keyboard.X; } break;
        case 'Y': { return &input->keyboard.Y; } break;
        case 'Z': { return &input->keyboard.Z; } break;
    }
    return nullptr;
}

void PressKey(Input* input, char key) {
    Button_State* button = GetKey(input, key);
    PressButton(button);
}

void LiftKey(Input* input, char key) {
    Button_State* button = GetKey(input, key);
    LiftButton(button);
}

#endif