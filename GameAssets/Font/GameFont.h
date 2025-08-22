#include "GamePlatform.h"
#include "GameMath.h"

// Freetype
// #include "ft2build.h"
// #include FT_FREETYPE_H

#ifndef GAME_FONTS
#define GAME_FONTS

#include <vector>

enum game_font_id {
    Font_Cascadia_Mono_ID,
    Font_Menlo_Regular_ID,

    game_font_id_count
};

const uint32 FONT_CHARACTERS_COUNT = '~' - ' ';
// const int LOAD_POINTS = 20

// Quantities in font design units
typedef int16 FWORD;
typedef uint16 UFWORD;

// Decimal type for fonts
typedef int16 F2DOT14;

struct composite_glyph_record {
    matrix2 Transform;
    float X, Y;
    char Child;
};

struct glyph_contour_point {
    uint32 Index;
    float X;
    float Y;
    bool OnCurve;
};

struct glyph_contour {
    glyph_contour_point* Points;
    uint16 Endpoint;
    uint16 nPoints;
    bool IsConvex;
    bool IsExterior;
};

struct game_font_character {
    char Letter;
    int16 nContours;
    int16 nPoints;
    uint16 nChildren;
    uint16 nOnCurve;
    uint16 nInteriorCurves;
    uint16 nExteriorCurves;
    glyph_contour* Contours;
    void* Data;
    uint32 VertexOffset;
    uint32 OutlineOffset;
    uint32 InteriorCurvesOffset;
    uint32 ExteriorCurvesOffset;
    uint32 SolidTrianglesOffset;
    int16 Left;
    int16 Top;
    uint16 Width;
    uint16 Height;
};

struct preprocessed_font {
    uint64 Size;
    read_file_result File;
    uint32 GlyphOffsets[FONT_CHARACTERS_COUNT];
    uint32 LocaOffset;
    uint32 GlyfOffset;
    uint16 nChildren[FONT_CHARACTERS_COUNT];
    uint16 GlyphIDs[FONT_CHARACTERS_COUNT];
    uint16 nPoints[FONT_CHARACTERS_COUNT];
    uint16 nOnCurvePoints[FONT_CHARACTERS_COUNT];
    uint32 nOnCurve;
    uint32 nTotalPoints;
    std::vector<glyph_contour> Contours[FONT_CHARACTERS_COUNT];
    uint16 nGlyphs;
    uint16 UnitsPerEm;
    uint16 LineJump;
    int16 IndexToLocFormat;
    UFWORD SpaceAdvance;
    UFWORD AdvanceWidths[FONT_CHARACTERS_COUNT];
    FWORD LeftSideBearings[FONT_CHARACTERS_COUNT];
    UFWORD AdvanceHeights[FONT_CHARACTERS_COUNT];
    FWORD TopSideBearings[FONT_CHARACTERS_COUNT];
};

struct game_font {
    game_font_id ID;
    uint16 SpaceAdvance;
    uint16 LineJump;
    float UnitsPerEm;
    game_font_character Characters[FONT_CHARACTERS_COUNT];
    uint32 nOnCurve;
    uint32 nPoints;
    float* Vertices;
    uint32* Elements;
};

float GetF2DOT14(F2DOT14 Number) {
    return Number / 16384.0f;
}

namespace ttf {

struct font_header {
    uint32 SFNTVersion;
    uint16 NumTables;
    uint16 SearchRange;
    uint16 EntrySelector;
    uint16 RangeShift;
};

font_header ParseTTFHeader(uint8* Memory) {
    font_header Result = *(font_header*)Memory;
    Result.SFNTVersion = BigEndian(Result.SFNTVersion);
    Result.NumTables = BigEndian(Result.NumTables);
    Result.SearchRange = BigEndian(Result.SearchRange);
    Result.EntrySelector = BigEndian(Result.EntrySelector);
    Result.RangeShift = BigEndian(Result.RangeShift);
    return Result;
}

struct table_record {
    char Tag[4];
    uint32 CheckSum;
    uint32 Offset;
    uint32 Length;
};

table_record ParseTTFTableRecord(uint8* Memory) {
    table_record Result = *(table_record*)Memory;
    Result.CheckSum = BigEndian(Result.CheckSum);
    Result.Offset = BigEndian(Result.Offset);
    Result.Length = BigEndian(Result.Length);
    return Result;
}

bool TagEquals(const char Tag[4], const char* Table) {
    return Tag[0] == Table[0] && Tag[1] == Table[1] && Tag[2] == Table[2] && Tag[3] == Table[3];
}

#pragma pack(push, 1)
struct head_table {
    uint32 Version;
    uint32 FontRevision;
    uint32 CheckSumAdjustment;
    uint32 MagicNumber;
    uint16 Flags;
    uint16 UnitsPerEm;
    int64 Created;
    int64 Modified;
    int16 MinX;
    int16 MinY;
    int16 MaxX;
    int16 MaxY;
    uint16 MacStyle;
    uint16 LowestRectPPEM;
    int16 FontDirectionHint;
    int16 IndexToLocFormat;
    int16 GlyphDataFormat;
};
#pragma pack(pop)

head_table ParseTTFHeadTable(uint8* Memory) {
    head_table Result = *(head_table*)Memory;
    Result.Version = BigEndian(Result.Version);
    Result.FontRevision = BigEndian(Result.FontRevision);
    Result.CheckSumAdjustment = BigEndian(Result.CheckSumAdjustment);
    Result.MagicNumber = BigEndian(Result.MagicNumber);
    Result.Flags = BigEndian(Result.Flags);
    Result.UnitsPerEm = BigEndian(Result.UnitsPerEm);
    Result.Created = BigEndian(Result.Created);
    Result.Modified = BigEndian(Result.Modified);
    Result.MinX = BigEndian(Result.MinX);
    Result.MinY = BigEndian(Result.MinY);
    Result.MaxX = BigEndian(Result.MaxX);
    Result.MaxY = BigEndian(Result.MaxY);
    Result.MacStyle = BigEndian(Result.MacStyle);
    Result.LowestRectPPEM = BigEndian(Result.LowestRectPPEM);
    Result.FontDirectionHint = BigEndian(Result.FontDirectionHint);
    Result.IndexToLocFormat = BigEndian(Result.IndexToLocFormat);
    Result.GlyphDataFormat = BigEndian(Result.GlyphDataFormat);
    return Result;
}

struct maxp_table {
    uint32 Version;
    uint16 NumGlyphs;
};

maxp_table ParseTTFMaxProfileTable(uint8* Memory) {
    maxp_table Result = *(maxp_table*)Memory;
    Result.Version = BigEndian(Result.Version);
    Result.NumGlyphs = BigEndian(Result.NumGlyphs);
    return Result;
}

struct os2_table {
    uint16	version;
    FWORD	xAvgCharWidth;
    uint16	usWeightClass;
    uint16	usWidthClass;
    uint16	fsType;
    FWORD	ySubscriptXSize;
    FWORD	ySubscriptYSize;
    FWORD	ySubscriptXOffset;
    FWORD	ySubscriptYOffset;
    FWORD	ySuperscriptXSize;
    FWORD	ySuperscriptYSize;
    FWORD	ySuperscriptXOffset;
    FWORD	ySuperscriptYOffset;
    FWORD	yStrikeoutSize;
    FWORD	yStrikeoutPosition;
    int16	sFamilyClass;
    uint8	panose[10];
    uint32	ulUnicodeRange1;
    uint32	ulUnicodeRange2;
    uint32	ulUnicodeRange3;
    uint32	ulUnicodeRange4;
    uint32	achVendID;
    uint16	fsSelection;
    uint16	usFirstCharIndex;
    uint16	usLastCharIndex;
    FWORD	sTypoAscender;
    FWORD	sTypoDescender;
    FWORD	sTypoLineGap;
    UFWORD	usWinAscent;
    UFWORD	usWinDescent;
    uint32	ulCodePageRange1;
    uint32	ulCodePageRange2;
    FWORD	sxHeight;
    FWORD	sCapHeight;
    uint16	usDefaultChar;
    uint16	usBreakChar;
    uint16	usMaxContext;
};

os2_table ParseTTFOS2Table(uint8* Memory) {
    os2_table Result = *(os2_table*)Memory;
    Result.version = BigEndian(Result.version);
    Result.xAvgCharWidth = BigEndian(Result.xAvgCharWidth);
    Result.usWeightClass = BigEndian(Result.usWeightClass);
    Result.usWidthClass = BigEndian(Result.usWidthClass);
    Result.fsType = BigEndian(Result.fsType);
    Result.ySubscriptXSize = BigEndian(Result.ySubscriptXSize);
    Result.ySubscriptYSize = BigEndian(Result.ySubscriptYSize);
    Result.ySubscriptXOffset = BigEndian(Result.ySubscriptXOffset);
    Result.ySubscriptYOffset = BigEndian(Result.ySubscriptYOffset);
    Result.ySuperscriptXSize = BigEndian(Result.ySuperscriptXSize);
    Result.ySuperscriptYSize = BigEndian(Result.ySuperscriptYSize);
    Result.ySuperscriptXOffset = BigEndian(Result.ySuperscriptXOffset);
    Result.ySuperscriptYOffset = BigEndian(Result.ySuperscriptYOffset);
    Result.yStrikeoutSize = BigEndian(Result.yStrikeoutSize);
    Result.yStrikeoutPosition = BigEndian(Result.yStrikeoutPosition);
    Result.sFamilyClass = BigEndian(Result.sFamilyClass);
    Result.panose[0] = BigEndian(Result.panose[0]);
    Result.panose[1] = BigEndian(Result.panose[1]);
    Result.panose[2] = BigEndian(Result.panose[2]);
    Result.panose[3] = BigEndian(Result.panose[3]);
    Result.panose[4] = BigEndian(Result.panose[4]);
    Result.panose[5] = BigEndian(Result.panose[5]);
    Result.panose[6] = BigEndian(Result.panose[6]);
    Result.panose[7] = BigEndian(Result.panose[7]);
    Result.panose[8] = BigEndian(Result.panose[8]);
    Result.panose[9] = BigEndian(Result.panose[9]);
    Result.ulUnicodeRange1 = BigEndian(Result.ulUnicodeRange1);
    Result.ulUnicodeRange2 = BigEndian(Result.ulUnicodeRange2);
    Result.ulUnicodeRange3 = BigEndian(Result.ulUnicodeRange3);
    Result.ulUnicodeRange4 = BigEndian(Result.ulUnicodeRange4);
    Result.achVendID = BigEndian(Result.achVendID);
    Result.fsSelection = BigEndian(Result.fsSelection);
    Result.usFirstCharIndex = BigEndian(Result.usFirstCharIndex);
    Result.usLastCharIndex = BigEndian(Result.usLastCharIndex);
    Result.sTypoAscender = BigEndian(Result.sTypoAscender);
    Result.sTypoDescender = BigEndian(Result.sTypoDescender);
    Result.sTypoLineGap = BigEndian(Result.sTypoLineGap);
    Result.usWinAscent = BigEndian(Result.usWinAscent);
    Result.usWinDescent = BigEndian(Result.usWinDescent);
    Result.ulCodePageRange1 = BigEndian(Result.ulCodePageRange1);
    Result.ulCodePageRange2 = BigEndian(Result.ulCodePageRange2);
    Result.sxHeight = BigEndian(Result.sxHeight);
    Result.sCapHeight = BigEndian(Result.sCapHeight);
    Result.usDefaultChar = BigEndian(Result.usDefaultChar);
    Result.usBreakChar = BigEndian(Result.usBreakChar);
    Result.usMaxContext = BigEndian(Result.usMaxContext);
    return Result;
}

struct cmap_header {
    uint16 Version;
    uint16 NumTables;
};

cmap_header ParseTTFCMapHeader(uint8* Memory) {
    cmap_header Result = *(cmap_header*)Memory;
    Result.Version = BigEndian(Result.Version);
    Result.NumTables = BigEndian(Result.NumTables);
    return Result;
}

struct encoding_record {
    uint16 PlatformID;
    uint16 EncodingID;
    uint32 Offset;            // From start of cmap table
};

encoding_record ParseTTFEncoding(uint8* Memory) {
    encoding_record Result = *(encoding_record*)Memory;
    Result.PlatformID = BigEndian(Result.PlatformID);
    Result.EncodingID = BigEndian(Result.EncodingID);
    Result.Offset = BigEndian(Result.Offset);
    return Result;
}

struct cmap_subtable {
    uint16 Format;
    uint16 Length;
    uint16 Language;
    uint16 SegCountX2;
    uint16 SearchRange;
    uint16 EntrySelector;
    uint16 RangeShift;
};

cmap_subtable ParseTTFCmapSubtable(uint8* Memory) {
    cmap_subtable Result = *(cmap_subtable*)Memory;
    Result.Format = BigEndian(Result.Format);
    Result.Length = BigEndian(Result.Length);
    Result.Language = BigEndian(Result.Language);
    Result.SegCountX2 = BigEndian(Result.SegCountX2);
    Result.SearchRange = BigEndian(Result.SearchRange);
    Result.EntrySelector = BigEndian(Result.EntrySelector);
    Result.RangeShift = BigEndian(Result.RangeShift);
    return Result;
}

struct glyph_header {
    int16 NumberOfContours;
    int16 MinX;
    int16 MinY;
    int16 MaxX;
    int16 MaxY;
};

glyph_header ParseTTFGlyphHeader(uint8* Memory) {
    glyph_header Result = *(glyph_header*)Memory;
    Result.NumberOfContours = BigEndian(Result.NumberOfContours);
    Result.MinX = BigEndian(Result.MinX);
    Result.MinY = BigEndian(Result.MinY);
    Result.MaxX = BigEndian(Result.MaxX);
    Result.MaxY = BigEndian(Result.MaxY);
    return Result;
}

struct hhead_table {
    uint16 MajorVersion;
    uint16 MinorVersion;
    FWORD Ascender;
    FWORD Descender;
    FWORD LineGap;
    UFWORD AdvanceWidthMax;
    FWORD MinLeftSideBearing;
    FWORD MinRightSideBearing;
    FWORD XMaxExtent;
    int16 CaretSlopeRise;
    int16 CaretSlopeRun;
    int16 CaretOffset;
    int16 Reserved[4];
    int16 MetricDataFormat;
    uint16 NumberOfHMetrics;
};

hhead_table ParseHorizontalHeadTable(uint8* Memory) {
    hhead_table Result = *(hhead_table*)Memory;
    Result.MajorVersion = BigEndian(Result.MajorVersion);
    Result.MinorVersion = BigEndian(Result.MinorVersion);
    Result.Ascender = BigEndian(Result.Ascender);
    Result.Descender = BigEndian(Result.Descender);
    Result.LineGap = BigEndian(Result.LineGap);
    Result.AdvanceWidthMax = BigEndian(Result.AdvanceWidthMax);
    Result.MinLeftSideBearing = BigEndian(Result.MinLeftSideBearing);
    Result.MinRightSideBearing = BigEndian(Result.MinRightSideBearing);
    Result.XMaxExtent = BigEndian(Result.XMaxExtent);
    Result.CaretSlopeRise = BigEndian(Result.CaretSlopeRise);
    Result.CaretSlopeRun = BigEndian(Result.CaretSlopeRun);
    Result.CaretOffset = BigEndian(Result.CaretOffset);
    Result.MetricDataFormat = BigEndian(Result.MetricDataFormat);
    Result.NumberOfHMetrics = BigEndian(Result.NumberOfHMetrics);
    return Result;
}

struct long_hor_metric {
    UFWORD AdvanceWidth;
    FWORD LeftSideBearing;
};

struct Version16Dot16 {
    uint16 Major;
    uint16 Minor;
};

struct vhead_table {
    Version16Dot16 Version;
    FWORD VertTypoAscender;
    FWORD VertTypoDescender;
    FWORD VertTypoLineGap;
    UFWORD AdvanceHeightMax;
    FWORD MinBottom;
    FWORD YMaxExtent;
    int16 CaretSlopeRise;
    int16 CaretSlopeRun;
    int16 CaretOffset;
    int16 Reserved[4];
    int16 MetricDataFormat;
    uint16 NumOfLongVerMetrics;
};

vhead_table ParseVerticalHeadTable(uint8* Memory) {
    vhead_table Result = *(vhead_table*)Memory;
    Result.Version.Major = BigEndian(Result.Version.Major);
    Result.Version.Minor = BigEndian(Result.Version.Minor);
    Result.VertTypoAscender = BigEndian(Result.VertTypoAscender);
    Result.VertTypoDescender = BigEndian(Result.VertTypoDescender);
    Result.VertTypoLineGap = BigEndian(Result.VertTypoLineGap);
    Result.AdvanceHeightMax = BigEndian(Result.AdvanceHeightMax);
    Result.MinBottom = BigEndian(Result.MinBottom);
    Result.YMaxExtent = BigEndian(Result.YMaxExtent);
    Result.CaretSlopeRise = BigEndian(Result.CaretSlopeRise);
    Result.CaretSlopeRun = BigEndian(Result.CaretSlopeRun);
    Result.CaretOffset = BigEndian(Result.CaretOffset);
    Result.MetricDataFormat = BigEndian(Result.MetricDataFormat);
    Result.NumOfLongVerMetrics = BigEndian(Result.NumOfLongVerMetrics);
    return Result;
}

struct vertical_metric {
    UFWORD AdvanceHeight;
    FWORD TopSideBearing;
};

enum simple_glyph_flag {
    ON_CURVE_POINT                       = 1 << 0,
    X_SHORT_VECTOR                       = 1 << 1,
    Y_SHORT_VECTOR                       = 1 << 2,
    REPEAT_FLAG                          = 1 << 3,
    X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR = 1 << 4,
    Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR = 1 << 5,
    OVERLAP_SIMPLE                       = 1 << 6,
};

enum composite_glyph_flag {
    ARG_1_AND_2_ARE_WORDS     = 1 << 0,
    ARGS_ARE_XY_VALUES        = 1 << 1,
    ROUND_XY_TO_GRID          = 1 << 2,
    WE_HAVE_A_SCALE           = 1 << 3,
    MORE_COMPONENTS           = 1 << 5,
    WE_HAVE_AN_X_AND_Y_SCALE  = 1 << 6,
    WE_HAVE_A_TWO_BY_TWO      = 1 << 7,
    WE_HAVE_INSTRUCTIONS      = 1 << 8,
    USE_MY_METRICS            = 1 << 9,
    OVERLAP_COMPOUND          = 1 << 10,
    SCALED_COMPONENT_OFFSET   = 1 << 11,
    UNSCALED_COMPONENT_OFFSET = 1 << 12,
};

}

const float DPI = 96.0f;

preprocessed_font PreprocessFont(read_file_result File);
game_font LoadFont(memory_arena* Arena, preprocessed_font* Font);
//glyph_triangulation ComputeTriangulation(memory_arena* Arena, game_font* Font);

float GetCharMaxHeight(game_font* Font, int Points) {
    float PixelsPerEm = Points * (DPI / 72.0f) / Font->UnitsPerEm;
    return Font->LineJump * PixelsPerEm;
}

void GetTextWidthAndHeight(const char* Text, game_font* Font, int Points, float* Width, float* Height) {
    float PixelsPerEm = Points * (DPI / 72.0f) / Font->UnitsPerEm;

    *Height = GetCharMaxHeight(Font, Points);
    *Width = 0;
    
    int Length = strlen(Text);
    for (int i = 0; i < Length; i++) {
        char c = Text[i];
        if (c == '#' && Text[i+1] == '#') break;
        if (c == ' ')             *Width += Font->SpaceAdvance * PixelsPerEm;
        if ('!' <= c && c <= '~') *Width += Font->Characters[c - '!'].Width * PixelsPerEm;
        if (c == '\n')            *Height += Font->LineJump * PixelsPerEm;
    }
}

#endif