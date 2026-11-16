#pragma once
#include "mathematics.h"

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Color                                                                                                                                        |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/

INTROSPECT
struct Color {
    float r;
    float g;
    float b;
    float a;

    static const Color EMPTY;
    static const Color BLACK;
    static const Color WHITE;
    static const Color GRAY;
    static const Color DARK_GRAY;
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color MAGENTA;
    static const Color YELLOW;
    static const Color CYAN;
    static const Color ORANGE;
    static const Color PURPLE;
};

Color ChangeAlpha(Color color, float alpha);
Color operator*(float luminosity, Color color);
uint32 GetColorBytes(Color color);
Color HSV2RGB(float h, float s, float v, float a);
Color Mix(Color color1, Color color2, float t);
