#pragma once

#include "platform.h"
#include "mathematics.h"

constexpr uint32 FONT_CHARACTERS_COUNT = '~' - ' ';

// Quantities in font design units
typedef int16 FWORD;
typedef uint16 UFWORD;

// Decimal type for fonts
typedef int16 F2DOT14;

struct Composite_Glyph_Record {
    Matrix2 transform;
    float x, y;
    char child;
};

INTROSPECT
struct Glyph_Contour_Point {
    uint32 index;
    float x;
    float y;
    bool on_curve;
};

INTROSPECT
struct Glyph_Contour {
    Glyph_Contour_Point* points;
    uint16 end_point;
    uint16 point_count;
    bool is_convex;
    bool is_exterior;
};

INTROSPECT
struct Game_Font_Character {
    char letter;
    int16 contour_count;
    int16 point_count;
    uint16 children_count;
    uint16 on_curve_count;
    uint16 interior_curve_count;
    uint16 exterior_curve_count;
    uint16 solid_triangle_count;
    Glyph_Contour* contours;
    void* data;
    uint32 vertex_offset;
    uint32 outline_offset;
    uint32 interior_curves_offset;
    uint32 exterior_curves_offset;
    uint32 solid_triangles_offset;
    int16 left;
    int16 top;
    uint16 width;
    uint16 height;
};

struct Preprocessed_Font {
    uint64 Size;
    uint32 GlyphOffsets[FONT_CHARACTERS_COUNT];
    uint32 LocaOffset;
    uint32 GlyfOffset;
    uint16 nChildren[FONT_CHARACTERS_COUNT];
    uint16 GlyphIDs[FONT_CHARACTERS_COUNT];
    uint16 nPoints[FONT_CHARACTERS_COUNT];
    uint16 nOnCurvePoints[FONT_CHARACTERS_COUNT];
    uint32 nOnCurve;
    uint32 nTotalPoints;
    // std::vector<Glyph_Contour> Contours[FONT_CHARACTERS_COUNT];
    uint16 nGlyphs;
    uint16 units_per_em;
    uint16 LineJump;
    int16 IndexToLocFormat;
    UFWORD space_advance;
    UFWORD AdvanceWidths[FONT_CHARACTERS_COUNT];
    FWORD LeftSideBearings[FONT_CHARACTERS_COUNT];
    UFWORD AdvanceHeights[FONT_CHARACTERS_COUNT];
    FWORD TopSideBearings[FONT_CHARACTERS_COUNT];
    int16 MinX;
    int16 Maxx;
    int16 MinY;
    int16 MaxY;
};

INTROSPECT
struct Font {
    String name;
    uint16 space_advance;
    uint16 line_jump;
    int16 min_x;
    int16 max_x;
    int16 min_y;
    int16 max_y;
    float units_per_em;
    Game_Font_Character characters[FONT_CHARACTERS_COUNT];
    uint32 on_curve_count;
    uint32 point_count;
    uint32 curve_triangle_count;
    uint32 solid_triangle_count;
    float* vertices;
    uint32* elements;
};
