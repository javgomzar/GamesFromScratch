#include "pch.h"
#include "color.h"


const Color Color::EMPTY     { 0.0f, 0.0f, 0.0f, 0.0f };
const Color Color::BLACK     { 0.0f, 0.0f, 0.0f, 1.0f };
const Color Color::WHITE     { 1.0f, 1.0f, 1.0f, 1.0f };
const Color Color::GRAY      { 0.5f, 0.5f, 0.5f, 1.0f };
const Color Color::DARK_GRAY { 0.1f, 0.1f, 0.1f, 1.0f };
const Color Color::RED       { 1.0f, 0.0f, 0.0f, 1.0f };
const Color Color::GREEN     { 0.0f, 1.0f, 0.0f, 1.0f };
const Color Color::BLUE      { 0.0f, 0.0f, 1.0f, 1.0f };
const Color Color::MAGENTA   { 1.0f, 0.0f, 1.0f, 1.0f };
const Color Color::YELLOW    { 1.0f, 1.0f, 0.0f, 1.0f };
const Color Color::CYAN      { 0.0f, 1.0f, 1.0f, 1.0f };
const Color Color::ORANGE    { 1.0f, 0.63f,0.0f, 1.0f };
const Color Color::PURPLE    { 0.5f, 0.0f, 0.6f, 1.0f };

Color ChangeAlpha(Color color, float alpha) {
    return Color(color.r, color.g, color.b, alpha);
}

Color operator*(float luminosity, Color color) {
    return {
        Clamp(luminosity * color.r, 0.0f, 1.0f),
        Clamp(luminosity * color.g, 0.0f, 1.0f),
        Clamp(luminosity * color.b, 0.0f, 1.0f),
        color.a
    };
}

uint32 GetColorBytes(Color color) {
    uint8 r = color.r * 255.0f;
    uint8 g = color.g * 255.0f;
    uint8 b = color.b * 255.0f;
    uint8 a = color.a * 255.0f;
    return (a << 24) | (r << 16) | (g << 8) | b;
}

Color HSV2RGB(float h, float s, float v, float a = 1.0f) {
    float r = (1.0f-s) + s*Clamp(fabsf(fmodf(h + 1.0f, 1.0f)      * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    float g = (1.0f-s) + s*Clamp(fabsf(fmodf(h + 2.0f/3.0f, 1.0f) * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    float b = (1.0f-s) + s*Clamp(fabsf(fmodf(h + 1.0f/3.0f, 1.0f) * 6.0f - 3.0f) - 1.0f, 0.0f, 1.0f);
    return v * Color(r, g, b, a);
}

Color Mix(Color color1, Color color2, float t) {
    return Color(
        (1-t)*color1.r + t*color2.r,
        (1-t)*color1.g + t*color2.g,
        (1-t)*color1.b + t*color2.b,
        (1-t)*color1.a + t*color2.a
    );
}
