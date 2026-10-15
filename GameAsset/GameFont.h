#ifndef GAME_FONT
#define GAME_FONT

#include "GamePlatform.h"
#include "GameMath.h"

#include <vector>

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

V2 GetContourPointV2(Glyph_Contour_Point* point) {
    return V2(point->x, point->y);
}

INTROSPECT
struct Glyph_Contour {
    Glyph_Contour_Point* points;
    uint16 end_point;
    uint16 point_count;
    bool is_convex;
    bool is_exterior;
};

INTROSPECT
struct Game_font_Character {
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

struct preprocessed_font {
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
    std::vector<Glyph_Contour> Contours[FONT_CHARACTERS_COUNT];
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
    Game_font_Character characters[FONT_CHARACTERS_COUNT];
    uint32 on_curve_count;
    uint32 point_count;
    uint32 curve_triangle_count;
    uint32 solid_triangle_count;
    float* vertices;
    uint32* elements;
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
    font_header result = *(font_header*)Memory;
    result.SFNTVersion = BigEndian(result.SFNTVersion);
    result.NumTables = BigEndian(result.NumTables);
    result.SearchRange = BigEndian(result.SearchRange);
    result.EntrySelector = BigEndian(result.EntrySelector);
    result.RangeShift = BigEndian(result.RangeShift);
    return result;
}

struct table_record {
    char Tag[4];
    uint32 checksum;
    uint32 offset;
    uint32 length;
};

table_record ParseTTFTableRecord(uint8* Memory) {
    table_record result = *(table_record*)Memory;
    result.checksum = BigEndian(result.checksum);
    result.offset = BigEndian(result.offset);
    result.length = BigEndian(result.length);
    return result;
}

bool TagEquals(const char tag[4], const char* table) {
    return tag[0] == table[0] && tag[1] == table[1] && tag[2] == table[2] && tag[3] == table[3];
}

#pragma pack(push, 1)
struct head_table {
    uint32 Version;
    uint32 fontRevision;
    uint32 CheckSumAdjustment;
    uint32 MagicNumber;
    uint16 Flags;
    uint16 units_per_em;
    int64 Created;
    int64 Modified;
    int16 MinX;
    int16 MinY;
    int16 Maxx;
    int16 MaxY;
    uint16 MacStyle;
    uint16 LowestRectPPEM;
    int16 fontDirectionHint;
    int16 IndexToLocFormat;
    int16 GlyphDataFormat;
};
#pragma pack(pop)

head_table ParseTTFHeadTable(uint8* Memory) {
    head_table result = *(head_table*)Memory;
    result.Version = BigEndian(result.Version);
    result.fontRevision = BigEndian(result.fontRevision);
    result.CheckSumAdjustment = BigEndian(result.CheckSumAdjustment);
    result.MagicNumber = BigEndian(result.MagicNumber);
    result.Flags = BigEndian(result.Flags);
    result.units_per_em = BigEndian(result.units_per_em);
    result.Created = BigEndian(result.Created);
    result.Modified = BigEndian(result.Modified);
    result.MinX = BigEndian(result.MinX);
    result.MinY = BigEndian(result.MinY);
    result.Maxx = BigEndian(result.Maxx);
    result.MaxY = BigEndian(result.MaxY);
    result.MacStyle = BigEndian(result.MacStyle);
    result.LowestRectPPEM = BigEndian(result.LowestRectPPEM);
    result.fontDirectionHint = BigEndian(result.fontDirectionHint);
    result.IndexToLocFormat = BigEndian(result.IndexToLocFormat);
    result.GlyphDataFormat = BigEndian(result.GlyphDataFormat);
    return result;
}

struct maxp_table {
    uint32 Version;
    uint16 NumGlyphs;
};

maxp_table ParseTTFMaxProfileTable(uint8* Memory) {
    maxp_table result = *(maxp_table*)Memory;
    result.Version = BigEndian(result.Version);
    result.NumGlyphs = BigEndian(result.NumGlyphs);
    return result;
}

enum fsSelection_flags {
    FS_ITALIC             = 1 << 0,
    FS_UNDERSCORE         = 1 << 1,
    FS_NEGATIVE           = 1 << 2,
    FS_OUTLINED           = 1 << 3,
    FS_STRIKEOUT          = 1 << 4,
    FS_BOLD               = 1 << 5,
    FS_REGULAR            = 1 << 6,
    FS_USE_TYPO_METRICS   = 1 << 7,
    FS_WEIGHT_WIDTH_SLOPE = 1 << 8,
    FS_OBLIQUE            = 1 << 9,
};

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
};

os2_table ParseTTFOS2Table(uint8* Memory) {
    os2_table result = *(os2_table*)Memory;
    result.version = BigEndian(result.version);
    result.xAvgCharWidth = BigEndian(result.xAvgCharWidth);
    result.usWeightClass = BigEndian(result.usWeightClass);
    result.usWidthClass = BigEndian(result.usWidthClass);
    result.fsType = BigEndian(result.fsType);
    result.ySubscriptXSize = BigEndian(result.ySubscriptXSize);
    result.ySubscriptYSize = BigEndian(result.ySubscriptYSize);
    result.ySubscriptXOffset = BigEndian(result.ySubscriptXOffset);
    result.ySubscriptYOffset = BigEndian(result.ySubscriptYOffset);
    result.ySuperscriptXSize = BigEndian(result.ySuperscriptXSize);
    result.ySuperscriptYSize = BigEndian(result.ySuperscriptYSize);
    result.ySuperscriptXOffset = BigEndian(result.ySuperscriptXOffset);
    result.ySuperscriptYOffset = BigEndian(result.ySuperscriptYOffset);
    result.yStrikeoutSize = BigEndian(result.yStrikeoutSize);
    result.yStrikeoutPosition = BigEndian(result.yStrikeoutPosition);
    result.sFamilyClass = BigEndian(result.sFamilyClass);
    result.panose[0] = BigEndian(result.panose[0]);
    result.panose[1] = BigEndian(result.panose[1]);
    result.panose[2] = BigEndian(result.panose[2]);
    result.panose[3] = BigEndian(result.panose[3]);
    result.panose[4] = BigEndian(result.panose[4]);
    result.panose[5] = BigEndian(result.panose[5]);
    result.panose[6] = BigEndian(result.panose[6]);
    result.panose[7] = BigEndian(result.panose[7]);
    result.panose[8] = BigEndian(result.panose[8]);
    result.panose[9] = BigEndian(result.panose[9]);
    result.ulUnicodeRange1 = BigEndian(result.ulUnicodeRange1);
    result.ulUnicodeRange2 = BigEndian(result.ulUnicodeRange2);
    result.ulUnicodeRange3 = BigEndian(result.ulUnicodeRange3);
    result.ulUnicodeRange4 = BigEndian(result.ulUnicodeRange4);
    result.achVendID = BigEndian(result.achVendID);
    result.fsSelection = BigEndian(result.fsSelection);
    result.usFirstCharIndex = BigEndian(result.usFirstCharIndex);
    result.usLastCharIndex = BigEndian(result.usLastCharIndex);
    result.sTypoAscender = BigEndian(result.sTypoAscender);
    result.sTypoDescender = BigEndian(result.sTypoDescender);
    result.sTypoLineGap = BigEndian(result.sTypoLineGap);
    result.usWinAscent = BigEndian(result.usWinAscent);
    result.usWinDescent = BigEndian(result.usWinDescent);
    return result;
}

struct cmap_header {
    uint16 Version;
    uint16 NumTables;
};

cmap_header ParseTTFCMapHeader(uint8* Memory) {
    cmap_header result = *(cmap_header*)Memory;
    result.Version = BigEndian(result.Version);
    result.NumTables = BigEndian(result.NumTables);
    return result;
}

struct encoding_record {
    uint16 PlatformID;
    uint16 EncodingID;
    uint32 Offset;            // From start of cmap table
};

encoding_record ParseTTFEncoding(uint8* Memory) {
    encoding_record result = *(encoding_record*)Memory;
    result.PlatformID = BigEndian(result.PlatformID);
    result.EncodingID = BigEndian(result.EncodingID);
    result.Offset = BigEndian(result.Offset);
    return result;
}

struct cmap_subtable {
    uint16 format;
    uint16 length;
    uint16 language;
    uint16 seg_countx2;
    uint16 search_range;
    uint16 entry_selector;
    uint16 range_shift;
};

cmap_subtable ParseTTFCmapSubtable(uint8* Memory) {
    cmap_subtable result = *(cmap_subtable*)Memory;
    result.format = BigEndian(result.format);
    result.length = BigEndian(result.length);
    result.language = BigEndian(result.language);
    result.seg_countx2 = BigEndian(result.seg_countx2);
    result.search_range = BigEndian(result.search_range);
    result.entry_selector = BigEndian(result.entry_selector);
    result.range_shift = BigEndian(result.range_shift);
    return result;
}

struct glyph_header {
    int16 NumberOfContours;
    int16 MinX;
    int16 MinY;
    int16 Maxx;
    int16 MaxY;
};

glyph_header ParseTTFGlyphHeader(uint8* Memory) {
    glyph_header result = *(glyph_header*)Memory;
    result.NumberOfContours = BigEndian(result.NumberOfContours);
    result.MinX = BigEndian(result.MinX);
    result.MinY = BigEndian(result.MinY);
    result.Maxx = BigEndian(result.Maxx);
    result.MaxY = BigEndian(result.MaxY);
    return result;
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
    hhead_table result = *(hhead_table*)Memory;
    result.MajorVersion = BigEndian(result.MajorVersion);
    result.MinorVersion = BigEndian(result.MinorVersion);
    result.Ascender = BigEndian(result.Ascender);
    result.Descender = BigEndian(result.Descender);
    result.LineGap = BigEndian(result.LineGap);
    result.AdvanceWidthMax = BigEndian(result.AdvanceWidthMax);
    result.MinLeftSideBearing = BigEndian(result.MinLeftSideBearing);
    result.MinRightSideBearing = BigEndian(result.MinRightSideBearing);
    result.XMaxExtent = BigEndian(result.XMaxExtent);
    result.CaretSlopeRise = BigEndian(result.CaretSlopeRise);
    result.CaretSlopeRun = BigEndian(result.CaretSlopeRun);
    result.CaretOffset = BigEndian(result.CaretOffset);
    result.MetricDataFormat = BigEndian(result.MetricDataFormat);
    result.NumberOfHMetrics = BigEndian(result.NumberOfHMetrics);
    return result;
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
    vhead_table result = *(vhead_table*)Memory;
    result.Version.Major = BigEndian(result.Version.Major);
    result.Version.Minor = BigEndian(result.Version.Minor);
    result.VertTypoAscender = BigEndian(result.VertTypoAscender);
    result.VertTypoDescender = BigEndian(result.VertTypoDescender);
    result.VertTypoLineGap = BigEndian(result.VertTypoLineGap);
    result.AdvanceHeightMax = BigEndian(result.AdvanceHeightMax);
    result.MinBottom = BigEndian(result.MinBottom);
    result.YMaxExtent = BigEndian(result.YMaxExtent);
    result.CaretSlopeRise = BigEndian(result.CaretSlopeRise);
    result.CaretSlopeRun = BigEndian(result.CaretSlopeRun);
    result.CaretOffset = BigEndian(result.CaretOffset);
    result.MetricDataFormat = BigEndian(result.MetricDataFormat);
    result.NumOfLongVerMetrics = BigEndian(result.NumOfLongVerMetrics);
    return result;
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

struct glyph_polygon {
	linked_list Vertices;
};

void ClosestPoints(glyph_polygon P, glyph_polygon Q, link** OutP, link** OutQ) {
	link* PVertex = P.Vertices.First;
	link* QVertex = Q.Vertices.First;
	float D = FLT_MAX;
	link* resultP = nullptr;
	link* resultQ = nullptr;
	do {
		do {
            V2 PPoint = GetContourPointV2((Glyph_Contour_Point*)PVertex->Data);
            V2 QPoint = GetContourPointV2((Glyph_Contour_Point*)QVertex->Data);

			float Candidate = distance(PPoint, QPoint);
			if (Candidate < D) {
				resultP = PVertex;
				resultQ = QVertex;
				D = Candidate;
			}

			QVertex = QVertex->Next;
		}
		while (QVertex && QVertex != Q.Vertices.First);
		PVertex = PVertex->Next;
	} while(PVertex && PVertex != P.Vertices.First);

	*OutP = resultP;
	*OutQ = resultQ;
}

/*
	Concatenates two polygons by their closest points: 
	Suppose a polygon `P` given by vertices `v_1, v_2, ..., v_m` and `Q` given by vertices `w_1, w_2, ..., w_n`. 
	Suppose `v_i` is the closest vertex in `P` to `Q` and `w_j` is the closest point in `Q` to `P`. 
	Then, the output of this function will be the polygon given by vertices 
	`v_j, ..., v_m, v_1, ..., v_j, w_i, ..., w_n, w_1, ..., w_i`.
*/
glyph_polygon Concatenate(Arena* Arena, glyph_polygon P, glyph_polygon Q) {
	glyph_polygon result = {};

	link* V = nullptr;
	link* W = nullptr;
	ClosestPoints(P, Q, &V, &W);

	result.Vertices.First = V;
	result.Vertices.Last = V->Previous;

	link* Link = PushStruct(Arena, link);
	Link->Data = result.Vertices.First->Data;
	result.Vertices.PushBack(Link);
	
	link* Previous = W->Previous;
	result.Vertices.PushBack(W);
	
	Link = PushStruct(Arena, link);
	Link->Data = W->Data;
	result.Vertices.Last = Previous;
	result.Vertices.PushBack(Link);
	
	result.Vertices.MakeCircular();
	return result;
}

uint64 CountVertices(glyph_polygon P) {
	return GetLength(P.Vertices);
}

float GetArea(glyph_polygon Polygon) {
	link* Link = Polygon.Vertices.First;
    Glyph_Contour_Point OPoint = *(Glyph_Contour_Point*)Link->Data;
	V2 O = V2(OPoint.x, OPoint.y);
	float result = 0;
	do {
		V2 P = GetContourPointV2((Glyph_Contour_Point*)Link->Data);
		V2 Q = GetContourPointV2((Glyph_Contour_Point*)Link->Next->Data);

		result += Q.x * P.y - P.x * Q.y;

		Link = Link->Next;
	} while (Link && Link != Polygon.Vertices.First);
	return result;
}

float Length(glyph_polygon Polygon) {
	link* Link = Polygon.Vertices.First;
	float result = 0;
	while (Link && Link != Polygon.Vertices.Last) {
		V2 P = GetContourPointV2((Glyph_Contour_Point*)Link->Data);
		V2 Q = GetContourPointV2((Glyph_Contour_Point*)Link->Next->Data);
	
		result += distance(P, Q);

		Link = Link->Next;
	}
	return result;
}

bool IsConvex(glyph_polygon Polygon) {
	link* Link = Polygon.Vertices.First;

	V2 A = GetContourPointV2((Glyph_Contour_Point*)Polygon.Vertices.Last->Data);
	V2 B = GetContourPointV2((Glyph_Contour_Point*)Link->Data);
	V2 C = GetContourPointV2((Glyph_Contour_Point*)Link->Next->Data);
	triangle2 FirstT = { A, B, C };
	float FirstArea = GetArea(FirstT);

	// In case first triangle is flat
	uint32 n = CountVertices(Polygon);
	uint32 i = 0;
	while (fabsf(FirstArea) <= EPSILON && i++ < n) {
		A = GetContourPointV2((Glyph_Contour_Point*)Link->Data);
		B = GetContourPointV2((Glyph_Contour_Point*)Link->Next->Data);
		C = GetContourPointV2((Glyph_Contour_Point*)Link->Next->Next->Data);

		FirstT = { A, B, C };
		FirstArea = GetArea(FirstT);

		if (fabsf(FirstArea) > EPSILON) {
			Link = Polygon.Vertices.First;
		}
	}

	// If the curve bends in different directions -> not convex
	while (Link && Link != Polygon.Vertices.Last) {
		A = GetContourPointV2((Glyph_Contour_Point*)Link->Data);
		B = GetContourPointV2((Glyph_Contour_Point*)Link->Next->Data);
		C = GetContourPointV2((Glyph_Contour_Point*)Link->Next->Next->Data);

		triangle2 T = { A, B, C };
		if (FirstArea*GetArea(T) < 0) return false;

		Link = Link->Next;
	}
	return true;
}

float SqDistance(glyph_polygon P, V2 Q) {
	link* Link = P.Vertices.First;
	V2 A = GetContourPointV2((Glyph_Contour_Point*)Link->Data);
	V2 B = GetContourPointV2((Glyph_Contour_Point*)Link->Next->Data);
	segment2 S = {A, B};
	float result = SqDistance(S, Q);
	Link = Link->Next;
	while (Link) {
		B = GetContourPointV2((Glyph_Contour_Point*)Link->Data);
		S.Head = S.Tail;
		S.Tail = B;
		float D = SqDistance(S, Q);
		if (D < result) result = D;
		if (Link == P.Vertices.Last) break;
		Link = Link->Next;
	}
	Link = P.Vertices.First;
	S.Head = S.Tail;
	S.Tail = GetContourPointV2((Glyph_Contour_Point*)Link->Data);
	float D = SqDistance(S, Q);
	if (D < result) result = D;

	return result;
}

float GetWindingNumber(glyph_polygon P, V2 Q) {
	float result = 0;
	uint64 n = CountVertices(P);
	link* Link = P.Vertices.First;
	do {
		V2 A = GetContourPointV2((Glyph_Contour_Point*)Link->Data);
		V2 B = GetContourPointV2((Glyph_Contour_Point*)Link->Next->Data);

		result += GetAngle(A - Q, B - Q);
		Link = Link->Next;
	}
	while (Link && Link != P.Vertices.First);
	
    return result;
}

bool IsInside(glyph_polygon P, V2 Q) {
	float Winding = GetWindingNumber(P, Q);
	return fabsf(Winding) > 0.1;
}

}

const float DPI = 96.0f;

float GetCharMaxHeight(Font* font, float Points) {
    float pixels_per_em = Points * (DPI / 72.0f) / font->units_per_em;
    return font->max_y * pixels_per_em;
}

void GetTextWidthAndHeight(String Text, Font* font, float Points, float* Width, float* Height) {
    float pixels_per_em = Points * (DPI / 72.0f) / font->units_per_em;

    float resultWidth = 0;
    float resultHeight = GetCharMaxHeight(font, Points);
    
    float LineWidth = 0;
    for (int i = 0; i < Text.length; i++) {
        char c = Text[i];
        if (c == '#' && Text[i+1] == '#') break;
        if (c == ' ') {
            LineWidth += font->space_advance * pixels_per_em;
        }
        if ('!' <= c && c <= '~') {
            LineWidth += font->characters[c - '!'].width * pixels_per_em;
        }
        if (c == '\n') {
            resultHeight += font->line_jump * pixels_per_em;
            if (LineWidth > resultWidth) resultWidth = LineWidth;
            LineWidth = 0;
        }
    }

    if (resultWidth == 0) {
        resultWidth = LineWidth;
    }

    resultHeight -= font->min_y * pixels_per_em;

    *Width  = resultWidth;
    *Height = resultHeight;
}

float GetTextWidth(String Text, Font* font, float Points) {
    float pixels_per_em = Points * (DPI / 72.0f) / font->units_per_em;

    float resultWidth = 0;
    
    float LineWidth = 0;
    for (int i = 0; i < Text.length; i++) {
        char c = Text[i];
        if (c == '#' && Text[i+1] == '#') break;
        if (c == ' ') {
            LineWidth += font->space_advance * pixels_per_em;
        }
        if ('!' <= c && c <= '~') {
            LineWidth += font->characters[c - '!'].width * pixels_per_em;
        }
        if (c == '\n') {
            if (LineWidth > resultWidth) resultWidth = LineWidth;
            LineWidth = 0;
        }
    }

    if (resultWidth == 0) {
        resultWidth = LineWidth;
    }

    return resultWidth;
}

// +------------------------------------------------------------------------------------------------------------------------------------------+
// | Utilities for TTF file parsing                                                                                                           |
// +------------------------------------------------------------------------------------------------------------------------------------------+

void FillGlyphOffsets(uint32* GlyphOffsets, uint32* LocationsTable, int16 IndexToLocFormat, uint16 nGlyphs) {
    switch(IndexToLocFormat) {
        case 0: {
            uint16* ShortLocationsTable = (uint16*)LocationsTable;
            for (int i = 0; i <= nGlyphs; i++) {
                GlyphOffsets[i] = (uint32)BigEndian(ShortLocationsTable[i]) << 1;
            }
        } break;
        case 1: {
            for (int i = 0; i <= nGlyphs; i++) {
                GlyphOffsets[i] = BigEndian(LocationsTable[i]);
            }
        } break;

        default: Raise("Invalid index to location format in TTF File.");
    }
}

uint16 NextTTFGlyphFlag(uint8*& pFlag) {
    static uint8 RepeatCounter = 0;
    bool Repeat = *pFlag & ttf::REPEAT_FLAG;
    if (Repeat) {
        if (RepeatCounter > 0) {
            RepeatCounter--;
            if (RepeatCounter == 0) {
                pFlag += 2;
                return 2;
            }
        }
        else {
            RepeatCounter = *(pFlag + 1);
        }
    }
    else {
        pFlag += 1;
        return 1;
    }
    return 0;
}

int16 GetTTFCoordinate(bool IsShort, bool RepeatOrPositive, int16 Last, uint8*& Pointer) {
    int16 result = Last;
    if (IsShort) {
        uint8 DeltaX = *Pointer++;
        if (RepeatOrPositive) {
            result += DeltaX;
        }
        else {
            result -= DeltaX;
        }
    }
    else if (!RepeatOrPositive) {
        int16 DeltaX = BigEndian(*(int16*)Pointer);
        result += DeltaX;
        Pointer += 2;
    }
    return result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------+
// | font preprocessing                                                                                                                       |
// +------------------------------------------------------------------------------------------------------------------------------------------+

preprocessed_font Preprocessfont(void* FileContent) {
    using namespace ttf;

    preprocessed_font result = {};
    uint8* FilePointer = (uint8*)FileContent;

    font_header Header = ParseTTFHeader(FilePointer);
    Assert(Header.SFNTVersion == 0x00010000);

    long_hor_metric* HorizontalMetricsTable = 0;
    vertical_metric* VerticalMetricsTable = 0;
    int16 IndexToLocFormat = 0;

    // hhea values
    uint16 nHMetrics = 0, nVMetrics = 0;
    FWORD HheaLineJump = 0;

    // cmap pointers
    uint16* StartCodes = 0;
    uint16* EndCodes = 0;
    int16* IDDeltas = 0;
    uint16* IDRangeOffsets = 0;
    uint16 SegCount = 0;
    uint16 nGlyphs = 0;

    uint8* Pointer = FilePointer + sizeof(font_header);
    
    // Getting font tables offsets
    for (int i = 0; i < Header.NumTables; i++) {
        table_record TableRecord = ParseTTFTableRecord(Pointer);

        if (TagEquals(TableRecord.Tag, "head")) {
            head_table Head = ParseTTFHeadTable(FilePointer + TableRecord.offset);
            Assert(Head.Version == 0x00010000 && Head.MagicNumber == 0x5F0F3CF5);
            IndexToLocFormat = Head.IndexToLocFormat;
            result.IndexToLocFormat = Head.IndexToLocFormat;
            result.units_per_em = Head.units_per_em;
            result.MinX = Head.MinX;
            result.Maxx = Head.Maxx;
            result.MinY = Head.MinY;
            result.MaxY = Head.MaxY;
        }
        else if (TagEquals(TableRecord.Tag, "maxp")) {
            maxp_table MaxP = ParseTTFMaxProfileTable(FilePointer + TableRecord.offset);
            Assert(MaxP.Version == 0x00010000);
            nGlyphs = MaxP.NumGlyphs;
            result.nGlyphs = MaxP.NumGlyphs;
        }
        else if (TagEquals(TableRecord.Tag, "loca")) {
            result.LocaOffset = TableRecord.offset;
        }
        else if (TagEquals(TableRecord.Tag, "glyf")) {
            result.GlyfOffset = TableRecord.offset;
        }
        else if (TagEquals(TableRecord.Tag, "hhea")) {
            hhead_table HHeadTable = ParseHorizontalHeadTable(FilePointer + TableRecord.offset);
            nHMetrics = HHeadTable.NumberOfHMetrics;
            HheaLineJump = HHeadTable.Ascender - HHeadTable.Descender + HHeadTable.LineGap;
        }
        else if (TagEquals(TableRecord.Tag, "hmtx")) {
            HorizontalMetricsTable = (long_hor_metric*)(FilePointer + TableRecord.offset);
        }
        else if (TagEquals(TableRecord.Tag, "vhea")) {
            vhead_table VHeadTable = ParseVerticalHeadTable(FilePointer + TableRecord.offset);
            nVMetrics = VHeadTable.NumOfLongVerMetrics;
        }
        else if (TagEquals(TableRecord.Tag, "vmtx")) {
            VerticalMetricsTable = (vertical_metric*)(FilePointer + TableRecord.offset);
        }
        else if (TagEquals(TableRecord.Tag, "OS/2")) {
            os2_table OS2 = ParseTTFOS2Table(FilePointer + TableRecord.offset);
            fsSelection_flags Flags = (fsSelection_flags)OS2.fsSelection;
            if (Flags & FS_USE_TYPO_METRICS) {
                result.LineJump = OS2.sTypoAscender - OS2.sTypoDescender + OS2.sTypoLineGap;
            }
        }
        else if (TagEquals(TableRecord.Tag, "cmap")) {
            uint8* CMapPointer = FilePointer + TableRecord.offset;
            cmap_header CMapHeader = ParseTTFCMapHeader(CMapPointer);

            uint8* EncodingsPointer = CMapPointer + sizeof(cmap_header);
            for (int j = 0; j < CMapHeader.NumTables; j++) {
                encoding_record Encoding = ParseTTFEncoding(EncodingsPointer);

                if (Encoding.PlatformID == 3 && Encoding.EncodingID == 1) {
                    cmap_subtable Subtable = ParseTTFCmapSubtable(CMapPointer + Encoding.Offset);
                    Assert(Subtable.format == 4);
                    SegCount = Subtable.seg_countx2 >> 1;

                    EndCodes = (uint16*)(CMapPointer + Encoding.Offset + sizeof(cmap_subtable));
                    Assert(EndCodes[SegCount-1] == 0xffff);

                    uint16* ReservedPad = EndCodes + SegCount;
                    Assert(*ReservedPad == 0);

                    StartCodes = ReservedPad + 1;
                    Assert(StartCodes[SegCount-1] == 0xffff);

                    IDDeltas = (int16*)(StartCodes + SegCount);
                    IDRangeOffsets = ((uint16*)IDDeltas) + SegCount;

                    break;
                }
                
                EncodingsPointer += sizeof(encoding_record);
            }

            if (SegCount == 0) Raise("PlatformID == 3 (Windows) and EncodingID == 1 (Unicode) wasn't found.");
        }

        Pointer += sizeof(table_record);
    }

    if (
        nGlyphs == 0 || 
        result.LocaOffset == 0 ||
        result.GlyfOffset == 0 || 
        nHMetrics == 0 || 
        HorizontalMetricsTable == 0
    ) {
        Raise("Some font table wasn't found");
    }

    if (result.LineJump == 0) {
        result.LineJump = HheaLineJump;
    }

    FWORD* OtherLeftSideBearings = (FWORD*)(HorizontalMetricsTable + nHMetrics);
    FWORD* OtherTopSideBearings = 0;
    if (nVMetrics != 0 && VerticalMetricsTable != 0) {
        OtherTopSideBearings = (FWORD*)(VerticalMetricsTable + nVMetrics);
    }

    // Locations of glyphs in file
    uint32* GlyphOffsets = new uint32[nGlyphs+1];
    uint32* LocationsTable = (uint32*)(FilePointer + result.LocaOffset);
    FillGlyphOffsets(GlyphOffsets, LocationsTable, IndexToLocFormat, nGlyphs);

    // Getting number of glyph contours and points (to compute size)
    uint32 TotalPoints = 0;
    uint32 TotalOnCurvePoints = 0;
    uint32 TotalContours = 0;
    uint32 TotalCompositeRecords = 0;
    uint8* GlyfTable = FilePointer + result.GlyfOffset;
    uint32 i = 0;
    for (char c = ' '; c <= '~'; c++) {
        uint16 StartCode = 0, EndCode = 0;
        for (; i < SegCount; i++) {
            StartCode = BigEndian(StartCodes[i]);
            EndCode = BigEndian(EndCodes[i]);
            if (StartCode <= c && c <= EndCode) {
                break;
            }
            else if (EndCode == 0xFFFF && StartCode == 0xFFFF) {
                char ErrorBuffer[64];
                sprintf(ErrorBuffer, "Character '%c' wasn't found in font.", c);
                Raise(ErrorBuffer);
            }
        }

        uint16 GlyphID;
        uint16 IDOffset = BigEndian(IDRangeOffsets[i]) / 2;
        if (IDOffset != 0) {
            uint16* GlyphIDPointer = IDRangeOffsets + i + (c - StartCode) + IDOffset;
            GlyphID = BigEndian(*GlyphIDPointer);
            if (GlyphID == 0) {
                char ErrorBuffer[64];
                sprintf(ErrorBuffer, "Character '%c' wasn't found in font.", c);
                Log(Log_Level::Error, ErrorBuffer);
            }
            else {
                GlyphID += BigEndian(IDDeltas[i]);
            }
        }
        else {
            GlyphID = c + BigEndian(IDDeltas[i]);
        }

        Assert(GlyphID > 0 && GlyphID < nGlyphs);
        if (c != ' ') result.GlyphIDs[c - '!'] = GlyphID;

        uint32 Offset = GlyphOffsets[GlyphID];
        uint32 GlyphLength = GlyphOffsets[GlyphID + 1] - Offset;
        uint8* GlyphData = GlyfTable + Offset;
        if (GlyphLength > 0) {
            glyph_header GlyphHeader = ParseTTFGlyphHeader(GlyphData);
            GlyphData += sizeof(glyph_header);

            if (GlyphHeader.NumberOfContours == 0) {
                Raise("No glyph contours found.");
            }
            
            // Composite glyphs
            else if (GlyphHeader.NumberOfContours < 0) {
                uint16* Pointer = (uint16*)(GlyphData);
                uint16 ChildGlyphIndex = 0;
                composite_glyph_flag Flags;
                uint32 nChildren = 0;
                do {
                    Flags = (composite_glyph_flag)BigEndian(*Pointer++);
                    ChildGlyphIndex = BigEndian(*(uint16*)(Pointer++));
                    nChildren++;

                    if (Flags & ARG_1_AND_2_ARE_WORDS) {
                        Pointer += 2;
                    }
                    else {
                        Pointer += 1;
                    }

                    if (Flags & WE_HAVE_A_SCALE) {
                        Pointer++;
                    }
                    else if (Flags & WE_HAVE_AN_X_AND_Y_SCALE) {
                        Pointer += 2;
                    }
                    else if (Flags & WE_HAVE_A_TWO_BY_TWO) {
                        Pointer += 4;
                    }
                } while(Flags & MORE_COMPONENTS);

                if (nChildren == 1) {
                    // If child is a simple glyph, treat this glyph as a simple glyph.
                    // When we load the glyph later, we will take care to get the child glyph and transform it
                    GlyphData = GlyfTable + GlyphOffsets[ChildGlyphIndex];

                    // For now only composite of simple glyphs are allowed (recursion depth = 1)
                    // TODO: Add composite of composite glyphs
                    GlyphHeader = ParseTTFGlyphHeader(GlyphData);
                    GlyphData += sizeof(glyph_header);
                    Assert(GlyphHeader.NumberOfContours > 0);
                }
                else if (nChildren > 1) {
                    TotalCompositeRecords += nChildren;
                }
                else {
                    Raise("Composite glyph with no children.");
                }

                result.nChildren[c - '!'] = nChildren;
            }
            
            // Simple glyphs
            if (GlyphHeader.NumberOfContours > 0) {
                TotalContours += GlyphHeader.NumberOfContours;
                result.Contours[c - '!'].resize(GlyphHeader.NumberOfContours);

                uint16* EndPtsOfContours = (uint16*)(GlyphData);
                uint16 CharacterDataPoints = BigEndian(EndPtsOfContours[GlyphHeader.NumberOfContours - 1]) + 1;

                uint16 InstructionLength = BigEndian(*(EndPtsOfContours + GlyphHeader.NumberOfContours));
                uint8* Instructions = (uint8*)(EndPtsOfContours + GlyphHeader.NumberOfContours + 1);
                uint8* pFlag = Instructions + InstructionLength;

                int j = 0;
                uint16 CharacterPoints = 0;
                uint16 CharacterOnCurvePoints = 0;
                for (int k = 0; k < GlyphHeader.NumberOfContours; k++) {
                    uint16 nPoints = 0;
                    simple_glyph_flag Flag = (simple_glyph_flag)*pFlag;
                    Assert(Flag & ON_CURVE_POINT);
                    bool PreviousOnCurve = false;
                    int Endpoint = BigEndian(EndPtsOfContours[k]);
                    for (; j <= Endpoint; j++) {
                        Flag = (simple_glyph_flag)*pFlag;
                        bool OnCurve = Flag & ON_CURVE_POINT;
                        if (!PreviousOnCurve && !OnCurve) {
                            nPoints += 1;
                            CharacterOnCurvePoints += 1;
                        }
                        else if (OnCurve) {
                            CharacterOnCurvePoints += 1;
                        }
                        nPoints += 1;
                        PreviousOnCurve = OnCurve;
                        NextTTFGlyphFlag(pFlag);
                    }
                    CharacterPoints += nPoints;
                    Glyph_Contour* Contour = &result.Contours[c - '!'][k];
                    Contour->point_count = nPoints;
                    Contour->end_point = CharacterPoints - 1;
                }

                result.nPoints[c - '!'] = CharacterPoints;
                result.nOnCurvePoints[c - '!'] = CharacterOnCurvePoints;
                TotalPoints += CharacterPoints;
                TotalOnCurvePoints += CharacterOnCurvePoints;
            }

            if (c != ' ') result.GlyphOffsets[c - '!'] = Offset;
        }
        else if (c != ' ') Raise("Current glyph has length 0.");
        
        if (c == ' ') {
            result.space_advance = BigEndian(HorizontalMetricsTable[GlyphID].AdvanceWidth);
        }
        else {
            // Horizontal metrics
            if (GlyphID < nHMetrics) {
                result.AdvanceWidths[c - '!']    = BigEndian(HorizontalMetricsTable[GlyphID].AdvanceWidth);
                result.LeftSideBearings[c - '!'] = BigEndian(HorizontalMetricsTable[GlyphID].LeftSideBearing);
            }
            else {
                result.AdvanceWidths[c - '!']    = BigEndian(HorizontalMetricsTable[nHMetrics-1].AdvanceWidth);
                result.LeftSideBearings[c - '!'] = BigEndian(OtherLeftSideBearings[GlyphID - nHMetrics]);
            }

            // Vertical metrics
            if (VerticalMetricsTable) {
                if (GlyphID < nVMetrics) {
                    result.AdvanceHeights[c - '!']  = BigEndian(VerticalMetricsTable[GlyphID].AdvanceHeight);
                    result.TopSideBearings[c - '!'] = BigEndian(VerticalMetricsTable[GlyphID].TopSideBearing);
                }
                else {
                    result.AdvanceHeights[c - '!'] = BigEndian(VerticalMetricsTable[nVMetrics - 1].AdvanceHeight);
                    result.TopSideBearings[c - '!'] = BigEndian(OtherTopSideBearings[GlyphID - nHMetrics]);
                }
            }
        }
    }

    delete [] GlyphOffsets;

    result.nOnCurve = TotalOnCurvePoints;

    result.Size = TotalContours * sizeof(Glyph_Contour);
    result.Size += TotalPoints * sizeof(Glyph_Contour_Point);
    result.Size += TotalCompositeRecords * sizeof(Composite_Glyph_Record);
    result.nTotalPoints = TotalPoints;

    return result;
}

/*
+------------------------------------------------------------------------------------------------------------------------------------------+
| font loading                                                                                                                             |
+------------------------------------------------------------------------------------------------------------------------------------------+
*/

Font LoadFont(
    Arena* Permanent, 
    Arena* Transient, 
    preprocessed_font* font,
    void* FileContent
) {
    using namespace ttf;

    Font result = {};
    result.space_advance = font->space_advance;
    result.units_per_em = font->units_per_em;
    result.line_jump = font->LineJump;
    result.on_curve_count = font->nOnCurve;
    result.point_count = font->nTotalPoints;
    result.min_x = font->MinX;
    result.max_x = font->Maxx;
    result.min_y = font->MinY;
    result.max_y = font->MaxY;

    uint8* FilePointer = (uint8*)FileContent;

    uint64 TransientStart = Transient->used;

    uint32* GlyphOffsets = PushArray(Transient, font->nGlyphs+1, uint32);
    uint32* LocationsTable = (uint32*)(FilePointer + font->LocaOffset);
    FillGlyphOffsets(GlyphOffsets, LocationsTable, font->IndexToLocFormat, font->nGlyphs);

    // Getting glyph data
    uint8* GlyfTable = FilePointer + font->GlyfOffset;
    for (char c = '!'; c <= '~'; c++) {
        Matrix2 Transform = identity2;
        V2 Translation = V2(0,0);

        uint32 Offset = font->GlyphOffsets[c - '!'];
        uint8* GlyphData = GlyfTable + Offset;
        glyph_header GlyphHeader = ParseTTFGlyphHeader(GlyphData);
        GlyphData += sizeof(glyph_header);

        Game_font_Character* Character = &result.characters[c - '!'];
        Character->letter         = c;
        Character->width          = font->AdvanceWidths[c - '!'];
        Character->height         = GlyphHeader.MaxY - GlyphHeader.MinY;
        Character->left           = font->LeftSideBearings[c - '!'];
        Character->top            = GlyphHeader.MaxY;
        Character->point_count    = font->nPoints[c - '!'];
        Character->on_curve_count = font->nOnCurvePoints[c - '!'];

        // Composite glyphs
        if (GlyphHeader.NumberOfContours < 0) {
            uint32 nChildren = font->nChildren[c - '!'];
            uint16* Pointer = (uint16*)GlyphData;
            Composite_Glyph_Record Record = {};

            if (nChildren == 0) Raise("Composite glyph with no children.");
            else if (nChildren > 1) {
                Character->children_count = nChildren;
            }

            composite_glyph_flag Flags;
            do {
                int16 X = 0, Y = 0;
                Flags = (composite_glyph_flag)BigEndian(*Pointer++);
                uint16 ChildGlyphIndex = BigEndian(*(uint16*)(Pointer++));

                if (Flags & ARGS_ARE_XY_VALUES) {
                    if (Flags & ARG_1_AND_2_ARE_WORDS) {
                        X = BigEndian(*(int16*)Pointer++);
                        Y = BigEndian(*(int16*)Pointer++);
                    }
                    else {
                        X = *(int8*)Pointer++;
                        Y = *(int8*)Pointer++;
                    }
                }
                else {
                    if (Flags & ARG_1_AND_2_ARE_WORDS) {
                        X = BigEndian(*(uint16*)Pointer++);
                        Y = BigEndian(*(uint16*)Pointer++);
                    }
                    else {
                        X = *(uint8*)Pointer++;
                        Y = *(uint8*)Pointer++;
                    }
                }

                if (Flags & WE_HAVE_A_SCALE) {
                    Transform.xx = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                    Transform.yy = Transform.xx;
                }
                else if (Flags & WE_HAVE_AN_X_AND_Y_SCALE) {
                    Transform.xx = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                    Transform.yy = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                }
                else if (Flags & WE_HAVE_A_TWO_BY_TWO) {
                    Transform.xx = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                    Transform.yx = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                    Transform.xy = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                    Transform.yy = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                }

                if (Flags & ARGS_ARE_XY_VALUES) {
                    if (Flags & ROUND_XY_TO_GRID) {
                        // TODO: Round to pixel grid
                        Raise("Not implemented");
                    }

                    if (Flags & SCALED_COMPONENT_OFFSET) {
                        V2 ComponentOffset = V2(X, Y);
                        Translation = Transform * ComponentOffset;
                    }
                    else {
                        Translation = V2(X, Y);
                    }
                }
                else {
                    // TODO: Point alignments
                    Raise("Not implemented");
                }

                if (Flags & USE_MY_METRICS) {
                    // TODO: Use the metrics of this component
                    Raise("Not implemented");
                }

                if (nChildren == 1) {
                    GlyphData = GlyfTable + GlyphOffsets[ChildGlyphIndex];
                    GlyphHeader = ParseTTFGlyphHeader(GlyphData);
                    GlyphData += sizeof(glyph_header);
                }
                else {
                    Composite_Glyph_Record* Record = PushStruct(Permanent, Composite_Glyph_Record);
                    char Found = 0;
                    for (char Child = '!'; Child <= '~'; Child++) {
                        if (font->GlyphIDs[Child - '!'] == ChildGlyphIndex) {
                            Found = Child;
                            break;
                        }
                    }
                    if (Found == 0) Raise("Child glyph was not one of the loaded glyphs");
                    Record->child = Found;
                    Record->x = Translation.x;
                    Record->y = Translation.y;
                    Record->transform = Transform;
                }
            } while(Flags & MORE_COMPONENTS);
        }

        // Sets the subglyph number of contours if glyph is composite with only one children
        Character->contour_count = GlyphHeader.NumberOfContours;

        // Simple glyphs
        if (GlyphHeader.NumberOfContours > 0) {
            uint16* EndPtsOfContours = (uint16*)GlyphData;
            Character->contours = PushArray(Permanent, Character->contour_count, Glyph_Contour);
            
            for (int i = 0; i < GlyphHeader.NumberOfContours; i++) {
                Character->contours[i] = font->Contours[c - '!'][i];
            }
    
            uint16 nPoints = BigEndian(EndPtsOfContours[GlyphHeader.NumberOfContours - 1]) + 1;
            Character->data = Permanent->base + Permanent->used;
    
            uint16 InstructionLength = BigEndian(*(EndPtsOfContours + GlyphHeader.NumberOfContours));
            uint8* Instructions = (uint8*)(EndPtsOfContours + GlyphHeader.NumberOfContours + 1);
            uint8* pFlag = Instructions + InstructionLength;
    
            uint16 nFlagBytes = 0, nXBytes = 0;
            for (int i = 0; i < nPoints; i++) {
                simple_glyph_flag Flag = (simple_glyph_flag)*pFlag;
                nFlagBytes += NextTTFGlyphFlag(pFlag);
                
                bool X_SHORT = Flag & X_SHORT_VECTOR;
                bool REPEAT_X = Flag & X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR;
    
                if (X_SHORT)        nXBytes++;
                else if (!REPEAT_X) nXBytes += 2;
            }
    
            pFlag = Instructions + InstructionLength;
            uint8* Xs = pFlag + nFlagBytes;
            uint8* Ys = Xs + nXBytes;
            int16 LastX = 0;
            int16 LastY = 0;
            int fontPointIndex = 0;
            uint8* MemoryLayoutStart = Permanent->base + Permanent->used;
            int TotalPoints = 0;
            for (int i = 0; i < GlyphHeader.NumberOfContours; i++) {
                Glyph_Contour* Contour = &Character->contours[i];
                Contour->points = (Glyph_Contour_Point*)(Permanent->base + Permanent->used);

                simple_glyph_flag Flag = (simple_glyph_flag)*pFlag;
                Assert(Flag & ON_CURVE_POINT);
                bool PreviousOnCurve = false;
                Contour->end_point = BigEndian(EndPtsOfContours[i]);

                bool Start = true;
                uint32 OutPointIndex = 0;
                linked_list Vertices = {};

                for (; fontPointIndex <= Contour->end_point; fontPointIndex++) {
                    Flag = (simple_glyph_flag)*pFlag;
                    bool XIsShort = Flag & X_SHORT_VECTOR;
                    bool YIsShort = Flag & Y_SHORT_VECTOR;
                    bool RepeatX = Flag & X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR;
                    bool RepeatY = Flag & Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR;
                    bool OnCurve = Flag & ON_CURVE_POINT;

                    int16 X = GetTTFCoordinate(XIsShort, RepeatX, LastX, Xs);
                    int16 Y = GetTTFCoordinate(YIsShort, RepeatY, LastY, Ys);

                    if (!PreviousOnCurve && !OnCurve) {
                        float MiddleX = 0.5f * (X + LastX);
                        float MiddleY = 0.5f * (Y + LastY);
                        Glyph_Contour_Point* result = PushArray(Permanent, 2, Glyph_Contour_Point);
                        result[0] = {
                            OutPointIndex++,
                            MiddleX, MiddleY,
                            true,
                        };
                        link* Link = PushStruct(Transient, link);
                        Link->Data = result;
                        Vertices.PushBack(Link);

                        result[1] = {
                            OutPointIndex++,
                            (float)X, (float)Y,
                            false
                        };

                        TotalPoints += 2;
                    }
                    else {
                        Glyph_Contour_Point* result = PushStruct(Permanent, Glyph_Contour_Point);
                        result[0] = {
                            OutPointIndex++,
                            (float)X, (float)Y,
                            OnCurve
                        };

                        if (OnCurve) {
                            link* Link = PushStruct(Transient, link);
                            Link->Data = result;
                            Vertices.PushBack(Link);
                        }

                        TotalPoints += 1;
                    }
                    
                    PreviousOnCurve = OnCurve;
                    LastX = X;
                    LastY = Y;
                    NextTTFGlyphFlag(pFlag);
                }
                Vertices.MakeCircular();

                // Post processing
                glyph_polygon Polygon = { Vertices };
                Contour->is_convex   = IsConvex(Polygon);
                Contour->is_exterior = GetArea(Polygon) > 0;
            }
            Assert(TotalPoints == font->nPoints[c - '!']);
            Assert(Permanent->base + Permanent->used == MemoryLayoutStart + sizeof(Glyph_Contour_Point) * font->nPoints[c - '!']);
        }
    }

    PopSize(Transient, Transient->used - TransientStart);

    return result;
}

uint32 WritefontVertices(Arena* Arena, Font* font) {
    uint32 Index = 0;
    font->vertices = (float*)(Arena->base + Arena->used);
    for (char c = '!'; c <= '~'; c++) {
        Game_font_Character* Character = &font->characters[c - '!'];
        Character->vertex_offset = Index;

        for (int i = 0; i < Character->contour_count; i++) {
            Glyph_Contour Contour = Character->contours[i];
            Glyph_Contour_Point Last = Contour.points[Contour.point_count-1];
            for (int j = 0; j < Contour.point_count; j++) {
                Glyph_Contour_Point* First = &Contour.points[j];
                if (First->on_curve) {
                    First->index = Index;
                    Glyph_Contour_Point Second = Contour.points[(j + 1) % Contour.point_count];
                    Glyph_Contour_Point Third = {};
                    if (Second.on_curve) {
                        Third = Second;
                        Second.x = 0.5f * (First->x + Third.x);
                        Second.y = 0.5f * (First->y + Third.y);
                    }
                    else {
                        Contour.points[(j + 1) % Contour.point_count].index = Index + 1;
                        Third = Contour.points[(j + 2) % Contour.point_count];
                    }

                    float* Out = PushArray(Arena, 12, float);
                    Out[0]  = First->x - 0.5f*Character->left;
                    Out[1]  = -First->y;
                    Out[2]  = 0.0f;
                    Out[3]  = 0.0f;
                    Out[4]  = Second.x - 0.5f*Character->left;
                    Out[5]  = -Second.y;
                    Out[6]  = 1.0f;
                    Out[7]  = 0.0f;
                    Out[8]  = Third.x - 0.5f*Character->left;
                    Out[9]  = -Third.y;
                    Out[10] = 0.0f;
                    Out[11] = 1.0f;

                    Index += 3;
                }
            }
        }
    }

    Assert(Index == 3 * font->on_curve_count);

    return Index;
}

void WritefontCurveTriangles(
    Arena* arena,
    Font* font
) {
    xarray<uint32> InteriorTriangles;
    xarray<uint32> ExteriorTriangles;

    font->elements = (uint32*)(arena->base + arena->used);
    uint32 Offset = 0;
    font->curve_triangle_count = 0;
    for (char c = '!'; c <= '~'; c++) {
        Game_font_Character* Character = &font->characters[c - '!'];
        Character->interior_curve_count = 0;
        Character->exterior_curve_count = 0;

        for (int i = 0; i < Character->contour_count; i++) {
            Glyph_Contour Contour = Character->contours[i];
            
            for (int j = 0; j < Contour.point_count; j++) {
                Glyph_Contour_Point* Point = &Contour.points[j];
                if (!Point->on_curve) {
                    Glyph_Contour_Point* Previous = &Contour.points[j-1];
                    Glyph_Contour_Point* Next = &Contour.points[(j+1) % Contour.point_count];

                    V2 A = V2(Previous->x, Previous->y);
                    V2 B = V2(Point->x, Point->y);
                    V2 C = V2(Next->x, Next->y);
                    triangle2 T = { A, B, C };
                    float Area = GetArea(T);

                    if (Area < 0) {
                        InteriorTriangles.Insert(Point->index-1);
                        InteriorTriangles.Insert(Point->index);
                        InteriorTriangles.Insert(Point->index+1);
                        Character->interior_curve_count += 1;
                    }
                    else if (Area > 0) {
                        ExteriorTriangles.Insert(Point->index-1);
                        ExteriorTriangles.Insert(Point->index);
                        ExteriorTriangles.Insert(Point->index+1);
                        Character->exterior_curve_count += 1;
                    }
                }
            }
        }

        Character->interior_curves_offset = Offset;
        if (Character->interior_curve_count > 0) {
            font->curve_triangle_count += Character->interior_curve_count;
            uint32* Elements = PushArray(arena, 3 * Character->interior_curve_count, uint32);
            for (int i = 0; i < 3 * Character->interior_curve_count; i++) {
                uint32 Element = InteriorTriangles[i];
                Elements[i] = Element;
            }
            Offset += 3 * Character->interior_curve_count;
        }

        Character->exterior_curves_offset = Offset;
        if (Character->exterior_curve_count > 0) {
            font->curve_triangle_count += Character->exterior_curve_count;
            uint32* Elements = PushArray(arena, 3 * Character->exterior_curve_count, uint32);
            for (int i = 0; i < 3 * Character->exterior_curve_count; i++) {
                uint32 Element = ExteriorTriangles[i];
                Elements[i] = Element;
            }
            Offset += 3 * Character->exterior_curve_count;
        }

        InteriorTriangles.Clear();
        ExteriorTriangles.Clear();
    }
}

void WritefontSolidTriangles(Arena* Permanent, Arena* Transient, Font* font) {
    using namespace ttf;

    uint32 Offset = 3 * font->curve_triangle_count;
    font->solid_triangle_count = 0;
    for (char c = '!'; c <= '~'; c++) {
        Game_font_Character* Character = &font->characters[c - '!'];
        Character->solid_triangles_offset = Offset;
        Character->solid_triangle_count = 0;

        for (int i = 0; i < Character->contour_count; i++) {
            Glyph_Contour Contour = Character->contours[i];
            
            if (Contour.is_exterior) {
                xarray<triangle2> VoidTriangles;
                glyph_polygon Polygon = {};

                // Add points in contour to the polygon
                for (int j = 0; j < Contour.point_count; j++) {
                    Glyph_Contour_Point* Point = &Contour.points[j];
                    Glyph_Contour_Point* Previous = &Contour.points[(j+Contour.point_count-1) % Contour.point_count];
                    if (!Previous->on_curve) {
                        Previous = &Contour.points[(j+Contour.point_count-2) % Contour.point_count];
                    }
                    Glyph_Contour_Point* Next = &Contour.points[(j+1) % Contour.point_count];
                    if (!Next->on_curve) {
                        Next = &Contour.points[(j+2) % Contour.point_count];
                    }

                    V2 A = GetContourPointV2(Previous);
                    V2 B = GetContourPointV2(Point);
                    V2 C = GetContourPointV2(Next);
                    triangle2 T = { A, B, C };
                    float Area = GetArea(T);

                    if (Area < 0) {
                        VoidTriangles.Insert(T);
                    } else if (!Point->on_curve) continue;

                    link* Link = PushStruct(Transient, link);
                    Link->Data = Point;
                    Polygon.Vertices.PushBack(Link);
                }
                uint32 nExteriorPoints = CountVertices(Polygon);
                Polygon.Vertices.MakeCircular();

                // We also need to add the interior contour points
                glyph_polygon InteriorPolygon = {};
                uint32 nInteriorContours = 0;
                uint32 nInteriorPoints = 0;
                for (int j = 0; j < Character->contour_count; j++) {
                    Glyph_Contour InteriorContour = Character->contours[j];

                    if (!InteriorContour.is_exterior) {
                        V2 TestPoint = V2(InteriorContour.points[0].x, InteriorContour.points[0].y);
                        if (IsInside(Polygon, TestPoint)) {
                            glyph_polygon NewInteriorPolygon = {};
                            for (int k = 0; k < InteriorContour.point_count; k++) {
                                link* Link = PushStruct(Transient, link);
                                Link->Data = &InteriorContour.points[k];
                                NewInteriorPolygon.Vertices.PushBack(Link);
                                V2 A = GetContourPointV2(&InteriorContour.points[(k+InteriorContour.point_count-1)%InteriorContour.point_count]);
                                V2 B = GetContourPointV2(&InteriorContour.points[k]);
                                V2 C = GetContourPointV2(&InteriorContour.points[(k+1)%InteriorContour.point_count]);
                                triangle2 T = {A,B,C};
                                float Area = GetArea(T);
                                if (Area < 0) {
                                    VoidTriangles.Insert(T);
                                }
                            }
                            NewInteriorPolygon.Vertices.MakeCircular();

                            if (nInteriorContours > 0) {
                                InteriorPolygon = Concatenate(Transient, InteriorPolygon, NewInteriorPolygon);
                            }
                            else {
                                InteriorPolygon = NewInteriorPolygon;
                            }

                            nInteriorPoints += InteriorContour.point_count;
                            nInteriorContours++;
                        }
                    }
                }
                if (nInteriorContours > 0) {
                    Polygon = Concatenate(Transient, Polygon, InteriorPolygon);
                }

                uint64 N = CountVertices(Polygon);
                uint64 VertexCount = N;
                for (int j = 0; j < N - 2; j++) {
                    Glyph_Contour_Point First = {}, Second = {}, Third = {};

                    link* Vertex = Polygon.Vertices.First;

                    bool Found = false;
                    bool EmptyTriangle = false;
                    do {
                        First  = *(Glyph_Contour_Point*)Vertex->Previous->Data;
                        Second = *(Glyph_Contour_Point*)Vertex->Data;
                        Third  = *(Glyph_Contour_Point*)Vertex->Next->Data;

                        V2 A = V2(First.x, First.y);
                        V2 B = V2(Second.x, Second.y);
                        V2 C = V2(Third.x, Third.y);

                        triangle2 T = { A, B, C };
                        float Area = GetArea(T);

                        if (Area == 0.0f) {
                            Found = true;
                            EmptyTriangle = true;
                            break;
                        }

                        if (Area > 0) {
                            bool Valid = true;
                            for (int k = 0; k < VoidTriangles.Size(); k++) {
                                triangle2 Void = VoidTriangles[k];

                                if (Intersect(Void, T)) {
                                    Valid = false;
                                    break;
                                }
                            }

                            if (Valid)  {
                                link* TestVertex = Polygon.Vertices.First;

                                segment2 Segments[3] = {
                                    {A, B},
                                    {B, C},
                                    {C, A}
                                };
                                do {
                                    Glyph_Contour_Point* ContourPoint = (Glyph_Contour_Point*)TestVertex->Data;
                                    if (
                                        ContourPoint->index != First.index && 
                                        ContourPoint->index != Second.index && 
                                        ContourPoint->index != Third.index
                                    ) {
                                        V2 P = GetContourPointV2(ContourPoint);
                                        // Test no other points of the contour are in the borders of the triangles: this can
                                        // cause some triangles to be invalid later
                                        if (IsInside(Segments[0], P) || IsInside(Segments[1], P) || IsInside(Segments[2], P)) {
                                            Valid = false;
                                            break;
                                        }
                                    }

                                    TestVertex = TestVertex->Next;
                                } while (TestVertex && TestVertex != Polygon.Vertices.First);

                                if (Valid) {
                                    Found = true;
                                    break;
                                }
                            }
                        }

                        Vertex = Vertex->Next;
                    } while (Vertex && Vertex != Polygon.Vertices.First);

                    if (!Found) {
                        if (VertexCount == 3) {
                            First  = *(Glyph_Contour_Point*)Vertex->Previous->Data;
                            Second = *(Glyph_Contour_Point*)Vertex->Data;
                            Third  = *(Glyph_Contour_Point*)Vertex->Next->Data;

                            V2 A = V2(First.x, First.y);
                            V2 B = V2(Second.x, Second.y);
                            V2 C = V2(Third.x, Third.y);

                            triangle2 T = { A, B, C };
                            float Area = GetArea(T);

                            if (Area > 0) {
                                // Ignore intersections with void triangles if it is the last one
                                uint32* Out = PushArray(Permanent, 3, uint32);
                                Out[0] = First.index;
                                Out[1] = Second.index;
                                Out[2] = Third.index;
            
                                Character->solid_triangle_count++;
    
                                Offset += 3;
                            }
                            break;
                        }
                    }

                    if (!EmptyTriangle) {
                        uint32* Out = PushArray(Permanent, 3, uint32);
                        Out[0] = First.index;
                        Out[1] = Second.index;
                        Out[2] = Third.index;
    
                        Character->solid_triangle_count++;
    
                        Offset += 3;
                    }
                
                    Polygon.Vertices.Break(Vertex);
                    VertexCount--;
                }

                font->solid_triangle_count += Character->solid_triangle_count;

                VoidTriangles.Clear();
            }
        }
    }
}

void Triangulatefont(Arena* fontsArena, Arena* Transient, Font* font) {
    WritefontVertices(fontsArena, font);
    WritefontCurveTriangles(fontsArena, font);
    WritefontSolidTriangles(fontsArena, Transient, font);
}

#endif