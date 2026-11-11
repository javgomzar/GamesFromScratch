#include "input.h"

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
