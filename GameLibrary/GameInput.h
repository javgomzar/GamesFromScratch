#ifndef GAME_INPUT
#define GAME_INPUT

#include "GamePlatform.h"
#include "GameMath.h"

// Platform independent structs and types
struct game_offscreen_buffer {
    void* Memory;
    uint32 Height;
    uint32 Width;
    uint32 Pitch;
    uint8 BytesPerPixel;
};

struct game_button_state {
    bool IsDown;
    bool WasDown;
    bool JustPressed;
    bool JustLifted;
};


// Joysticks values should be floats between 0 and 1
/*struct joystick_coordinate {
    float Start;
    float Max;
    float Min;
    float End;
};*/

const int NUMBER_OF_CONTROLLER_BUTTONS = 16;

struct game_controller_input {
    bool Any;
    v2 LeftJoystick;
    v2 RightJoystick;
    union {
        game_button_state Buttons[NUMBER_OF_CONTROLLER_BUTTONS];
        struct {
            game_button_state AButton;
            game_button_state BButton;
            game_button_state XButton;
            game_button_state YButton;
            game_button_state PadUp;
            game_button_state PadDown;
            game_button_state PadLeft;
            game_button_state PadRight;
            game_button_state RB;
            game_button_state LB;
            game_button_state RS;
            game_button_state LS;
            game_button_state RT;
            game_button_state LT;
            game_button_state Start;
            game_button_state Back;
        };
    };
};

const int NUMBER_OF_KEYS = 57;

struct game_keyboard_input {
    union {
        game_button_state Keys[NUMBER_OF_KEYS];
        struct {
            game_button_state One;
            game_button_state Two;
            game_button_state Three;
            game_button_state Four;
            game_button_state Five;
            game_button_state Six;
            game_button_state Seven;
            game_button_state Eight;
            game_button_state Nine;
            game_button_state Zero;
            game_button_state Q;
            game_button_state W;
            game_button_state E;
            game_button_state R;
            game_button_state T;
            game_button_state Y;
            game_button_state U;
            game_button_state I;
            game_button_state O;
            game_button_state P;
            game_button_state A;
            game_button_state S;
            game_button_state D;
            game_button_state F;
            game_button_state G;
            game_button_state H;
            game_button_state J;
            game_button_state K;
            game_button_state L;
            game_button_state Z;
            game_button_state X;
            game_button_state C;
            game_button_state V;
            game_button_state B;
            game_button_state N;
            game_button_state M;
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state Escape;
            game_button_state Space;
            game_button_state Enter;
            game_button_state Shift;
            game_button_state F1;
            game_button_state F2;
            game_button_state F3;
            game_button_state F4;
            game_button_state F5;
            game_button_state F6;
            game_button_state F7;
            game_button_state F8;
            game_button_state F9;
            game_button_state F10;
            game_button_state F11;
            game_button_state F12;
            game_button_state PageUp;
            game_button_state PageDown;
        };
    };
    bool Any;
};

struct game_mouse_input {
    game_button_state LeftClick;
    game_button_state MiddleClick;
    game_button_state RightClick;
    v2 Cursor;
    v2 LastCursor;
    short Wheel;
};

enum game_input_mode {
    Keyboard,
    Controller
};

struct game_input {
    game_input_mode Mode;
    game_controller_input Controller;
    game_keyboard_input Keyboard;
    game_mouse_input Mouse;
};

void UpdatePreviousInput(game_input* Input) {
// Mouse
    Input->Mouse.LeftClick.WasDown = Input->Mouse.LeftClick.IsDown;
    Input->Mouse.LeftClick.JustPressed = false;
    Input->Mouse.LeftClick.JustLifted = false;
    Input->Mouse.MiddleClick.WasDown = Input->Mouse.MiddleClick.IsDown;
    Input->Mouse.MiddleClick.JustPressed = false;
    Input->Mouse.MiddleClick.JustLifted = false;
    Input->Mouse.RightClick.WasDown = Input->Mouse.RightClick.IsDown;
    Input->Mouse.RightClick.JustPressed = false;
    Input->Mouse.RightClick.JustLifted = false;
    Input->Mouse.Wheel = 0;

// Keyboard
    for (int i = 0; i < NUMBER_OF_KEYS; i++) {
        game_button_state* Key = &Input->Keyboard.Keys[i];
        Key->WasDown = Key->IsDown;
        Key->JustPressed = false;
        Key->JustLifted = false;
    }

// Controller
    for (int i = 0; i < 16; i++) {
        game_button_state* Button = &Input->Controller.Buttons[i];
        Button->WasDown = Button->IsDown;
        Button->JustPressed = false;
        Button->JustLifted = false;
    }
}

void PressButton(game_button_state* Button) {
    Button->IsDown = true;
    Button->JustPressed = !Button->WasDown;
}

void LiftButton(game_button_state* Button) {
    Button->IsDown = false;
    Button->JustLifted = Button->WasDown;
}

game_button_state* GetKey(game_input* Input, char Key) {
    switch(Key) {
        case '0': { return &Input->Keyboard.Zero; } break;
        case '1': { return &Input->Keyboard.One; } break;
        case '2': { return &Input->Keyboard.Two; } break;
        case '3': { return &Input->Keyboard.Three; } break;
        case '4': { return &Input->Keyboard.Four; } break;
        case '5': { return &Input->Keyboard.Five; } break;
        case '6': { return &Input->Keyboard.Six; } break;
        case '7': { return &Input->Keyboard.Seven; } break;
        case '8': { return &Input->Keyboard.Eight; } break;
        case '9': { return &Input->Keyboard.Nine; } break;
        case 'A': { return &Input->Keyboard.A; } break;
        case 'B': { return &Input->Keyboard.B; } break;
        case 'C': { return &Input->Keyboard.C; } break;
        case 'D': { return &Input->Keyboard.D; } break;
        case 'E': { return &Input->Keyboard.E; } break;
        case 'F': { return &Input->Keyboard.F; } break;
        case 'G': { return &Input->Keyboard.G; } break;
        case 'H': { return &Input->Keyboard.H; } break;
        case 'I': { return &Input->Keyboard.I; } break;
        case 'J': { return &Input->Keyboard.J; } break;
        case 'K': { return &Input->Keyboard.K; } break;
        case 'L': { return &Input->Keyboard.L; } break;
        case 'M': { return &Input->Keyboard.M; } break;
        case 'N': { return &Input->Keyboard.N; } break;
        case 'O': { return &Input->Keyboard.O; } break;
        case 'P': { return &Input->Keyboard.P; } break;
        case 'Q': { return &Input->Keyboard.Q; } break;
        case 'R': { return &Input->Keyboard.R; } break;
        case 'S': { return &Input->Keyboard.S; } break;
        case 'T': { return &Input->Keyboard.T; } break;
        case 'U': { return &Input->Keyboard.U; } break;
        case 'V': { return &Input->Keyboard.V; } break;
        case 'W': { return &Input->Keyboard.W; } break;
        case 'X': { return &Input->Keyboard.X; } break;
        case 'Y': { return &Input->Keyboard.Y; } break;
        case 'Z': { return &Input->Keyboard.Z; } break;
        default: Assert(false);
    }
    return 0;
}

void PressKey(game_input* Input, char Key) {
    game_button_state* Button = GetKey(Input, Key);
    PressButton(Button);
}

void LiftKey(game_input* Input, char Key) {
    game_button_state* Button = GetKey(Input, Key);
    LiftButton(Button);
}

#endif