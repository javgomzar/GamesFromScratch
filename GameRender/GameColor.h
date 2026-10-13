#ifndef GAME_COLOR
#define GAME_COLOR

#include "GameMath.h"

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Color                                                                                                                                        |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/

struct Color {
    float R;
    float G;
    float B;
    float A;

    constexpr Color() : R(0), G(0), B(0), A(1) {};
    constexpr Color(float R, float G, float B, float A = 1.0f) : R(R), G(G), B(B), A(A) {};

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

inline constexpr Color Color::EMPTY     { 0.0f, 0.0f, 0.0f, 0.0f };
inline constexpr Color Color::BLACK     { 0.0f, 0.0f, 0.0f, 1.0f };
inline constexpr Color Color::WHITE     { 1.0f, 1.0f, 1.0f, 1.0f };
inline constexpr Color Color::GRAY      { 0.5f, 0.5f, 0.5f, 1.0f };
inline constexpr Color Color::DARK_GRAY { 0.1f, 0.1f, 0.1f, 1.0f };
inline constexpr Color Color::RED       { 1.0f, 0.0f, 0.0f, 1.0f };
inline constexpr Color Color::GREEN     { 0.0f, 1.0f, 0.0f, 1.0f };
inline constexpr Color Color::BLUE      { 0.0f, 0.0f, 1.0f, 1.0f };
inline constexpr Color Color::MAGENTA   { 1.0f, 0.0f, 1.0f, 1.0f };
inline constexpr Color Color::YELLOW    { 1.0f, 1.0f, 0.0f, 1.0f };
inline constexpr Color Color::CYAN      { 0.0f, 1.0f, 1.0f, 1.0f };
inline constexpr Color Color::ORANGE    { 1.0f, 0.63f, 0.0f, 1.0f };
inline constexpr Color Color::PURPLE    { 0.5f, 0.0f, 0.6f, 1.0f };

Color ChangeAlpha(Color color, float alpha) {
    return Color(color.R, color.G, color.B, alpha);
}

Color operator*(float Luminosity, Color color) {
    return {
        Clamp(Luminosity * color.R, 0.0f, 1.0f),
        Clamp(Luminosity * color.G, 0.0f, 1.0f),
        Clamp(Luminosity * color.B, 0.0f, 1.0f),
        color.A
    };
}

uint32 GetColorBytes(Color color) {
    uint8 R = color.R * 255.0f;
    uint8 G = color.G * 255.0f;
    uint8 B = color.B * 255.0f;
    uint8 A = color.A * 255.0f;
    return (A << 24) | (R << 16) | (G << 8) | B;
}

Color HSV2RGB(float H, float S, float V, float A = 1.0f) {
    float R = (1.0f-S) + S*Clamp(fabsf(fmodf(H + 1.0f, 1.0f)      * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    float G = (1.0f-S) + S*Clamp(fabsf(fmodf(H + 2.0f/3.0f, 1.0f) * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    float B = (1.0f-S) + S*Clamp(fabsf(fmodf(H + 1.0f/3.0f, 1.0f) * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    return V * Color(R, G, B, A);
}

Color Mix(Color color1, Color color2, float t) {
    return Color(
        (1-t)*color1.R + t*color2.R,
        (1-t)*color1.G + t*color2.G,
        (1-t)*color1.B + t*color2.B,
        (1-t)*color1.A + t*color2.A
    );
}

#endif