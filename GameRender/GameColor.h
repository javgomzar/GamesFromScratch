#ifndef GAME_COLOR
#define GAME_COLOR

#include "GameMath.h"

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Color                                                                                                                                        |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/

struct color {
    float R;
    float G;
    float B;
    float A;

    constexpr color() : R(0), G(0), B(0), A(1) {};
    constexpr color(float R, float G, float B, float A = 1.0f) : R(R), G(G), B(B), A(A) {};

    static const color EMPTY;
    static const color BLACK;
    static const color WHITE;
    static const color GRAY;
    static const color DARK_GRAY;
    static const color RED;
    static const color GREEN;
    static const color BLUE;
    static const color MAGENTA;
    static const color YELLOW;
    static const color CYAN;
    static const color ORANGE;
    static const color PURPLE;
};

inline constexpr color color::EMPTY     { 0.0f, 0.0f, 0.0f, 0.0f };
inline constexpr color color::BLACK     { 0.0f, 0.0f, 0.0f, 1.0f };
inline constexpr color color::WHITE     { 1.0f, 1.0f, 1.0f, 1.0f };
inline constexpr color color::GRAY      { 0.5f, 0.5f, 0.5f, 1.0f };
inline constexpr color color::DARK_GRAY { 0.1f, 0.1f, 0.1f, 1.0f };
inline constexpr color color::RED       { 1.0f, 0.0f, 0.0f, 1.0f };
inline constexpr color color::GREEN     { 0.0f, 1.0f, 0.0f, 1.0f };
inline constexpr color color::BLUE      { 0.0f, 0.0f, 1.0f, 1.0f };
inline constexpr color color::MAGENTA   { 1.0f, 0.0f, 1.0f, 1.0f };
inline constexpr color color::YELLOW    { 1.0f, 1.0f, 0.0f, 1.0f };
inline constexpr color color::CYAN      { 0.0f, 1.0f, 1.0f, 1.0f };
inline constexpr color color::ORANGE    { 1.0f, 0.63f, 0.0f, 1.0f };
inline constexpr color color::PURPLE    { 0.5f, 0.0f, 0.6f, 1.0f };

color ChangeAlpha(color Color, float Alpha) {
    return color(Color.R, Color.G, Color.B, Alpha);
}

color operator*(float Luminosity, color Color) {
    return {
        Clamp(Luminosity * Color.R, 0.0f, 1.0f),
        Clamp(Luminosity * Color.G, 0.0f, 1.0f),
        Clamp(Luminosity * Color.B, 0.0f, 1.0f),
        Color.A
    };
}

uint32 GetColorBytes(color Color) {
    uint8 R = Color.R * 255.0f;
    uint8 G = Color.G * 255.0f;
    uint8 B = Color.B * 255.0f;
    uint8 A = Color.A * 255.0f;
    return (A << 24) | (R << 16) | (G << 8) | B;
}

color HSV2RGB(float H, float S, float V, float A = 1.0f) {
    float R = (1.0f-S) + S*Clamp(fabsf(fmodf(H + 1.0f, 1.0f)      * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    float G = (1.0f-S) + S*Clamp(fabsf(fmodf(H + 2.0f/3.0f, 1.0f) * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    float B = (1.0f-S) + S*Clamp(fabsf(fmodf(H + 1.0f/3.0f, 1.0f) * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    return V * color(R, G, B, A);
}

color Mix(color Color1, color Color2, float t) {
    return color(
        (1-t)*Color1.R + t*Color2.R,
        (1-t)*Color1.G + t*Color2.G,
        (1-t)*Color1.B + t*Color2.B,
        (1-t)*Color1.A + t*Color2.A
    );
}

#endif