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
};


// Joysticks values should be floats between 0 and 1
/*struct joystick_coordinate {
    float Start;
    float Max;
    float Min;
    float End;
};*/

struct game_controller_input {
    bool Any;
    v2 LeftJoystick;
    v2 RightJoystick;
    union {
        game_button_state Buttons[16];
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
    bool Any;
    union
    {
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
};

struct game_mouse_input {
    game_button_state LeftClick;
    game_button_state MiddleClick;
    game_button_state RightClick;
    v3 Cursor;
    v3 LastCursor;
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
    Input->Mouse.MiddleClick.WasDown = Input->Mouse.MiddleClick.IsDown;
    Input->Mouse.RightClick.WasDown = Input->Mouse.RightClick.IsDown;
    Input->Mouse.Wheel = 0;

// Keyboard
    Input->Keyboard.One.WasDown = Input->Keyboard.One.IsDown;
    Input->Keyboard.Two.WasDown = Input->Keyboard.Two.IsDown;
    Input->Keyboard.Three.WasDown = Input->Keyboard.Three.IsDown;
    Input->Keyboard.Four.WasDown = Input->Keyboard.Four.IsDown;
    Input->Keyboard.Five.WasDown = Input->Keyboard.Five.IsDown;
    Input->Keyboard.Six.WasDown = Input->Keyboard.Six.IsDown;
    Input->Keyboard.Seven.WasDown = Input->Keyboard.Seven.IsDown;
    Input->Keyboard.Eight.WasDown = Input->Keyboard.Eight.IsDown;
    Input->Keyboard.Nine.WasDown = Input->Keyboard.Nine.IsDown;
    Input->Keyboard.Zero.WasDown = Input->Keyboard.Zero.IsDown;
    Input->Keyboard.Q.WasDown = Input->Keyboard.Q.IsDown;
    Input->Keyboard.W.WasDown = Input->Keyboard.W.IsDown;
    Input->Keyboard.E.WasDown = Input->Keyboard.E.IsDown;
    Input->Keyboard.R.WasDown = Input->Keyboard.R.IsDown;
    Input->Keyboard.T.WasDown = Input->Keyboard.T.IsDown;
    Input->Keyboard.Y.WasDown = Input->Keyboard.Y.IsDown;
    Input->Keyboard.U.WasDown = Input->Keyboard.U.IsDown;
    Input->Keyboard.I.WasDown = Input->Keyboard.I.IsDown;
    Input->Keyboard.O.WasDown = Input->Keyboard.O.IsDown;
    Input->Keyboard.P.WasDown = Input->Keyboard.P.IsDown;
    Input->Keyboard.A.WasDown = Input->Keyboard.A.IsDown;
    Input->Keyboard.S.WasDown = Input->Keyboard.S.IsDown;
    Input->Keyboard.D.WasDown = Input->Keyboard.D.IsDown;
    Input->Keyboard.F.WasDown = Input->Keyboard.F.IsDown;
    Input->Keyboard.G.WasDown = Input->Keyboard.G.IsDown;
    Input->Keyboard.H.WasDown = Input->Keyboard.H.IsDown;
    Input->Keyboard.J.WasDown = Input->Keyboard.J.IsDown;
    Input->Keyboard.K.WasDown = Input->Keyboard.K.IsDown;
    Input->Keyboard.L.WasDown = Input->Keyboard.L.IsDown;
    Input->Keyboard.Z.WasDown = Input->Keyboard.Z.IsDown;
    Input->Keyboard.X.WasDown = Input->Keyboard.X.IsDown;
    Input->Keyboard.C.WasDown = Input->Keyboard.C.IsDown;
    Input->Keyboard.V.WasDown = Input->Keyboard.V.IsDown;
    Input->Keyboard.B.WasDown = Input->Keyboard.B.IsDown;
    Input->Keyboard.N.WasDown = Input->Keyboard.N.IsDown;
    Input->Keyboard.M.WasDown = Input->Keyboard.M.IsDown;
    Input->Keyboard.Up.WasDown = Input->Keyboard.Up.IsDown;
    Input->Keyboard.Down.WasDown = Input->Keyboard.Down.IsDown;
    Input->Keyboard.Left.WasDown = Input->Keyboard.Left.IsDown;
    Input->Keyboard.Right.WasDown = Input->Keyboard.Right.IsDown;
    Input->Keyboard.Escape.WasDown = Input->Keyboard.Escape.IsDown;
    Input->Keyboard.Space.WasDown = Input->Keyboard.Space.IsDown;
    Input->Keyboard.Enter.WasDown = Input->Keyboard.Enter.IsDown;
    Input->Keyboard.F1.WasDown = Input->Keyboard.F1.IsDown;
    Input->Keyboard.F2.WasDown = Input->Keyboard.F2.IsDown;
    Input->Keyboard.F3.WasDown = Input->Keyboard.F3.IsDown;
    Input->Keyboard.F4.WasDown = Input->Keyboard.F4.IsDown;
    Input->Keyboard.F5.WasDown = Input->Keyboard.F5.IsDown;
    Input->Keyboard.F6.WasDown = Input->Keyboard.F6.IsDown;
    Input->Keyboard.F7.WasDown = Input->Keyboard.F7.IsDown;
    Input->Keyboard.F8.WasDown = Input->Keyboard.F8.IsDown;
    Input->Keyboard.F9.WasDown = Input->Keyboard.F9.IsDown;
    Input->Keyboard.F10.WasDown = Input->Keyboard.F10.IsDown;
    Input->Keyboard.F11.WasDown = Input->Keyboard.F11.IsDown;
    Input->Keyboard.F12.WasDown = Input->Keyboard.F12.IsDown;
    Input->Keyboard.PageUp.WasDown = Input->Keyboard.PageUp.IsDown;
    Input->Keyboard.PageDown.WasDown = Input->Keyboard.PageDown.IsDown;
    Input->Keyboard.Shift.WasDown = Input->Keyboard.Shift.IsDown;

// Controller
    Input->Controller.PadUp.WasDown = Input->Controller.PadUp.IsDown;
    Input->Controller.PadDown.WasDown = Input->Controller.PadDown.IsDown;
    Input->Controller.PadLeft.WasDown = Input->Controller.PadLeft.IsDown;
    Input->Controller.PadRight.WasDown = Input->Controller.PadRight.IsDown;
    Input->Controller.LB.WasDown = Input->Controller.LB.IsDown;
    Input->Controller.RB.WasDown = Input->Controller.RB.IsDown;
    Input->Controller.AButton.WasDown = Input->Controller.AButton.IsDown;
    Input->Controller.BButton.WasDown = Input->Controller.BButton.IsDown;
    Input->Controller.XButton.WasDown = Input->Controller.XButton.IsDown;
    Input->Controller.YButton.WasDown = Input->Controller.YButton.IsDown;
    Input->Controller.Start.WasDown = Input->Controller.Start.IsDown;
    Input->Controller.Back.WasDown = Input->Controller.Back.IsDown;
    Input->Controller.LS.WasDown = Input->Controller.LS.IsDown;
    Input->Controller.RS.WasDown = Input->Controller.RS.IsDown;
    Input->Controller.LT.WasDown = Input->Controller.LT.IsDown;
    Input->Controller.RT.WasDown = Input->Controller.RT.IsDown;
}

void PressKey(game_input* Input, char Key) {
    switch(Key) {
        case '0': { Input->Keyboard.Zero.IsDown = true; } break;
        case '1': { Input->Keyboard.One.IsDown = true; } break;
        case '2': { Input->Keyboard.Two.IsDown = true; } break;
        case '3': { Input->Keyboard.Three.IsDown = true; } break;
        case '4': { Input->Keyboard.Four.IsDown = true; } break;
        case '5': { Input->Keyboard.Five.IsDown = true; } break;
        case '6': { Input->Keyboard.Six.IsDown = true; } break;
        case '7': { Input->Keyboard.Seven.IsDown = true; } break;
        case '8': { Input->Keyboard.Eight.IsDown = true; } break;
        case '9': { Input->Keyboard.Nine.IsDown = true; } break;
        case 'A': { Input->Keyboard.A.IsDown = true; } break;
        case 'B': { Input->Keyboard.B.IsDown = true; } break;
        case 'C': { Input->Keyboard.C.IsDown = true; } break;
        case 'D': { Input->Keyboard.D.IsDown = true; } break;
        case 'E': { Input->Keyboard.E.IsDown = true; } break;
        case 'F': { Input->Keyboard.F.IsDown = true; } break;
        case 'G': { Input->Keyboard.G.IsDown = true; } break;
        case 'H': { Input->Keyboard.H.IsDown = true; } break;
        case 'I': { Input->Keyboard.I.IsDown = true; } break;
        case 'J': { Input->Keyboard.J.IsDown = true; } break;
        case 'K': { Input->Keyboard.K.IsDown = true; } break;
        case 'L': { Input->Keyboard.L.IsDown = true; } break;
        case 'M': { Input->Keyboard.M.IsDown = true; } break;
        case 'N': { Input->Keyboard.N.IsDown = true; } break;
        case 'O': { Input->Keyboard.O.IsDown = true; } break;
        case 'P': { Input->Keyboard.P.IsDown = true; } break;
        case 'Q': { Input->Keyboard.Q.IsDown = true; } break;
        case 'R': { Input->Keyboard.R.IsDown = true; } break;
        case 'S': { Input->Keyboard.S.IsDown = true; } break;
        case 'T': { Input->Keyboard.T.IsDown = true; } break;
        case 'U': { Input->Keyboard.U.IsDown = true; } break;
        case 'V': { Input->Keyboard.V.IsDown = true; } break;
        case 'W': { Input->Keyboard.W.IsDown = true; } break;
        case 'X': { Input->Keyboard.X.IsDown = true; } break;
        case 'Y': { Input->Keyboard.Y.IsDown = true; } break;
        case 'Z': { Input->Keyboard.Z.IsDown = true; } break;
    }
}

void LiftKey(game_input* Input, char Key) {
    switch(Key) {
        case '0': { Input->Keyboard.Zero.IsDown = false; } break;
        case '1': { Input->Keyboard.One.IsDown = false; } break;
        case '2': { Input->Keyboard.Two.IsDown = false; } break;
        case '3': { Input->Keyboard.Three.IsDown = false; } break;
        case '4': { Input->Keyboard.Four.IsDown = false; } break;
        case '5': { Input->Keyboard.Five.IsDown = false; } break;
        case '6': { Input->Keyboard.Six.IsDown = false; } break;
        case '7': { Input->Keyboard.Seven.IsDown = false; } break;
        case '8': { Input->Keyboard.Eight.IsDown = false; } break;
        case '9': { Input->Keyboard.Nine.IsDown = false; } break;
        case 'A': { Input->Keyboard.A.IsDown = false; } break;
        case 'B': { Input->Keyboard.B.IsDown = false; } break;
        case 'C': { Input->Keyboard.C.IsDown = false; } break;
        case 'D': { Input->Keyboard.D.IsDown = false; } break;
        case 'E': { Input->Keyboard.E.IsDown = false; } break;
        case 'F': { Input->Keyboard.F.IsDown = false; } break;
        case 'G': { Input->Keyboard.G.IsDown = false; } break;
        case 'H': { Input->Keyboard.H.IsDown = false; } break;
        case 'I': { Input->Keyboard.I.IsDown = false; } break;
        case 'J': { Input->Keyboard.J.IsDown = false; } break;
        case 'K': { Input->Keyboard.K.IsDown = false; } break;
        case 'L': { Input->Keyboard.L.IsDown = false; } break;
        case 'M': { Input->Keyboard.M.IsDown = false; } break;
        case 'N': { Input->Keyboard.N.IsDown = false; } break;
        case 'O': { Input->Keyboard.O.IsDown = false; } break;
        case 'P': { Input->Keyboard.P.IsDown = false; } break;
        case 'Q': { Input->Keyboard.Q.IsDown = false; } break;
        case 'R': { Input->Keyboard.R.IsDown = false; } break;
        case 'S': { Input->Keyboard.S.IsDown = false; } break;
        case 'T': { Input->Keyboard.T.IsDown = false; } break;
        case 'U': { Input->Keyboard.U.IsDown = false; } break;
        case 'V': { Input->Keyboard.V.IsDown = false; } break;
        case 'W': { Input->Keyboard.W.IsDown = false; } break;
        case 'X': { Input->Keyboard.X.IsDown = false; } break;
        case 'Y': { Input->Keyboard.Y.IsDown = false; } break;
        case 'Z': { Input->Keyboard.Z.IsDown = false; } break;
    }
}

#endif