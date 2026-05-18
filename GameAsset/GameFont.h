#include "GamePlatform.h"
#include "GameMath.h"

#ifndef GAME_FONT
#define GAME_FONT

#include <vector>

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

v2 GetContourPointV2(glyph_contour_point* ContourPoint) {
    return V2(ContourPoint->X, ContourPoint->Y);
}

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
    uint16 nSolidTriangles;
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
    file_info FileInfo;
    void* FileContent;
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
    int16 MinX;
    int16 MaxX;
    int16 MinY;
    int16 MaxY;
};

struct game_font {
    game_font_id ID;
    uint16 SpaceAdvance;
    uint16 LineJump;
    int16 MinX;
    int16 MaxX;
    int16 MinY;
    int16 MaxY;
    float UnitsPerEm;
    game_font_character Characters[FONT_CHARACTERS_COUNT];
    uint32 nOnCurve;
    uint32 nPoints;
    uint32 nCurveTriangles;
    uint32 nSolidTriangles;
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

struct glyph_polygon {
	linked_list Vertices;
};

void ClosestPoints(glyph_polygon P, glyph_polygon Q, link** OutP, link** OutQ) {
	link* PVertex = P.Vertices.First;
	link* QVertex = Q.Vertices.First;
	float D = FLT_MAX;
	link* ResultP = nullptr;
	link* ResultQ = nullptr;
	do {
		do {
            v2 PPoint = GetContourPointV2((glyph_contour_point*)PVertex->Data);
            v2 QPoint = GetContourPointV2((glyph_contour_point*)QVertex->Data);

			float Candidate = distance(PPoint, QPoint);
			if (Candidate < D) {
				ResultP = PVertex;
				ResultQ = QVertex;
				D = Candidate;
			}

			QVertex = QVertex->Next;
		}
		while (QVertex && QVertex != Q.Vertices.First);
		PVertex = PVertex->Next;
	} while(PVertex && PVertex != P.Vertices.First);

	*OutP = ResultP;
	*OutQ = ResultQ;
}

/*
	Concatenates two polygons by their closest points: 
	Suppose a polygon `P` given by vertices `v_1, v_2, ..., v_m` and `Q` given by vertices `w_1, w_2, ..., w_n`. 
	Suppose `v_i` is the closest vertex in `P` to `Q` and `w_j` is the closest point in `Q` to `P`. 
	Then, the output of this function will be the polygon given by vertices 
	`v_j, ..., v_m, v_1, ..., v_j, w_i, ..., w_n, w_1, ..., w_i`.
*/
glyph_polygon Concatenate(memory_arena* Arena, glyph_polygon P, glyph_polygon Q) {
	glyph_polygon Result = {};

	link* V = nullptr;
	link* W = nullptr;
	ClosestPoints(P, Q, &V, &W);

	Result.Vertices.First = V;
	Result.Vertices.Last = V->Previous;

	link* Link = PushStruct(Arena, link);
	Link->Data = Result.Vertices.First->Data;
	Result.Vertices.PushBack(Link);
	
	link* Previous = W->Previous;
	Result.Vertices.PushBack(W);
	
	Link = PushStruct(Arena, link);
	Link->Data = W->Data;
	Result.Vertices.Last = Previous;
	Result.Vertices.PushBack(Link);
	
	Result.Vertices.MakeCircular();
	return Result;
}

uint64 CountVertices(glyph_polygon P) {
	return GetLength(P.Vertices);
}

float GetArea(glyph_polygon Polygon) {
	link* Link = Polygon.Vertices.First;
    glyph_contour_point OPoint = *(glyph_contour_point*)Link->Data;
	v2 O = V2(OPoint.X, OPoint.Y);
	float Result = 0;
	do {
		v2 P = GetContourPointV2((glyph_contour_point*)Link->Data);
		v2 Q = GetContourPointV2((glyph_contour_point*)Link->Next->Data);

		Result += Q.X * P.Y - P.X * Q.Y;

		Link = Link->Next;
	} while (Link && Link != Polygon.Vertices.First);
	return Result;
}

float Length(glyph_polygon Polygon) {
	link* Link = Polygon.Vertices.First;
	float Result = 0;
	while (Link && Link != Polygon.Vertices.Last) {
		v2 P = GetContourPointV2((glyph_contour_point*)Link->Data);
		v2 Q = GetContourPointV2((glyph_contour_point*)Link->Next->Data);
	
		Result += distance(P, Q);

		Link = Link->Next;
	}
	return Result;
}

bool IsConvex(glyph_polygon Polygon) {
	link* Link = Polygon.Vertices.First;

	v2 A = GetContourPointV2((glyph_contour_point*)Polygon.Vertices.Last->Data);
	v2 B = GetContourPointV2((glyph_contour_point*)Link->Data);
	v2 C = GetContourPointV2((glyph_contour_point*)Link->Next->Data);
	triangle2 FirstT = { A, B, C };
	float FirstArea = GetArea(FirstT);

	// In case first triangle is flat
	uint32 n = CountVertices(Polygon);
	uint32 i = 0;
	while (fabsf(FirstArea) <= Epsilon && i++ < n) {
		A = GetContourPointV2((glyph_contour_point*)Link->Data);
		B = GetContourPointV2((glyph_contour_point*)Link->Next->Data);
		C = GetContourPointV2((glyph_contour_point*)Link->Next->Next->Data);

		FirstT = { A, B, C };
		FirstArea = GetArea(FirstT);

		if (fabsf(FirstArea) > Epsilon) {
			Link = Polygon.Vertices.First;
		}
	}

	// If the curve bends in different directions -> not convex
	while (Link && Link != Polygon.Vertices.Last) {
		A = GetContourPointV2((glyph_contour_point*)Link->Data);
		B = GetContourPointV2((glyph_contour_point*)Link->Next->Data);
		C = GetContourPointV2((glyph_contour_point*)Link->Next->Next->Data);

		triangle2 T = { A, B, C };
		if (FirstArea*GetArea(T) < 0) return false;

		Link = Link->Next;
	}
	return true;
}

float SqDistance(glyph_polygon P, v2 Q) {
	link* Link = P.Vertices.First;
	v2 A = GetContourPointV2((glyph_contour_point*)Link->Data);
	v2 B = GetContourPointV2((glyph_contour_point*)Link->Next->Data);
	segment2 S = {A, B};
	float Result = SqDistance(S, Q);
	Link = Link->Next;
	while (Link) {
		B = GetContourPointV2((glyph_contour_point*)Link->Data);
		S.Head = S.Tail;
		S.Tail = B;
		float D = SqDistance(S, Q);
		if (D < Result) Result = D;
		if (Link == P.Vertices.Last) break;
		Link = Link->Next;
	}
	Link = P.Vertices.First;
	S.Head = S.Tail;
	S.Tail = GetContourPointV2((glyph_contour_point*)Link->Data);
	float D = SqDistance(S, Q);
	if (D < Result) Result = D;

	return Result;
}

float GetWindingNumber(glyph_polygon P, v2 Q) {
	float Result = 0;
	uint64 n = CountVertices(P);
	link* Link = P.Vertices.First;
	do {
		v2 A = GetContourPointV2((glyph_contour_point*)Link->Data);
		v2 B = GetContourPointV2((glyph_contour_point*)Link->Next->Data);

		Result += GetAngle(A - Q, B - Q);
		Link = Link->Next;
	}
	while (Link && Link != P.Vertices.First);
	
    return Result;
}

bool IsInside(glyph_polygon P, v2 Q) {
	float Winding = GetWindingNumber(P, Q);
	return fabsf(Winding) > 0.1;
}

}

const float DPI = 96.0f;

preprocessed_font PreprocessFont(file_info FileInfo);
game_font LoadFont(memory_arena* Arena, preprocessed_font* Font);
//glyph_triangulation ComputeTriangulation(memory_arena* Arena, game_font* Font);

float GetCharMaxHeight(game_font* Font, float Points) {
    float PixelsPerEm = Points * (DPI / 72.0f) / Font->UnitsPerEm;
    return Font->MaxY * PixelsPerEm;
}

void GetTextWidthAndHeight(string Text, game_font* Font, float Points, float* Width, float* Height) {
    float PixelsPerEm = Points * (DPI / 72.0f) / Font->UnitsPerEm;

    float ResultWidth = 0;
    float ResultHeight = GetCharMaxHeight(Font, Points);
    
    float LineWidth = 0;
    for (int i = 0; i < Text.Length; i++) {
        char c = Text[i];
        if (c == '#' && Text[i+1] == '#') break;
        if (c == ' ') {
            LineWidth += Font->SpaceAdvance * PixelsPerEm;
        }
        if ('!' <= c && c <= '~') {
            LineWidth += Font->Characters[c - '!'].Width * PixelsPerEm;
        }
        if (c == '\n') {
            ResultHeight += Font->LineJump * PixelsPerEm;
            if (LineWidth > ResultWidth) ResultWidth = LineWidth;
            LineWidth = 0;
        }
    }

    if (ResultWidth == 0) {
        ResultWidth = LineWidth;
    }

    ResultHeight -= Font->MinY * PixelsPerEm;

    *Width  = ResultWidth;
    *Height = ResultHeight;
}

float GetTextWidth(string Text, game_font* Font, float Points) {
    float PixelsPerEm = Points * (DPI / 72.0f) / Font->UnitsPerEm;

    float ResultWidth = 0;
    
    float LineWidth = 0;
    for (int i = 0; i < Text.Length; i++) {
        char c = Text[i];
        if (c == '#' && Text[i+1] == '#') break;
        if (c == ' ') {
            LineWidth += Font->SpaceAdvance * PixelsPerEm;
        }
        if ('!' <= c && c <= '~') {
            LineWidth += Font->Characters[c - '!'].Width * PixelsPerEm;
        }
        if (c == '\n') {
            if (LineWidth > ResultWidth) ResultWidth = LineWidth;
            LineWidth = 0;
        }
    }

    if (ResultWidth == 0) {
        ResultWidth = LineWidth;
    }

    return ResultWidth;
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
    int16 Result = Last;
    if (IsShort) {
        uint8 DeltaX = *Pointer++;
        if (RepeatOrPositive) {
            Result += DeltaX;
        }
        else {
            Result -= DeltaX;
        }
    }
    else if (!RepeatOrPositive) {
        int16 DeltaX = BigEndian(*(int16*)Pointer);
        Result += DeltaX;
        Pointer += 2;
    }
    return Result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------+
// | Font preprocessing                                                                                                                       |
// +------------------------------------------------------------------------------------------------------------------------------------------+

preprocessed_font PreprocessFont(file_info FileInfo, void* FileContent) {
    using namespace ttf;

    preprocessed_font Result = {};
    Result.FileContent = FileContent;
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
            head_table Head = ParseTTFHeadTable(FilePointer + TableRecord.Offset);
            Assert(Head.Version == 0x00010000 && Head.MagicNumber == 0x5F0F3CF5);
            IndexToLocFormat = Head.IndexToLocFormat;
            Result.IndexToLocFormat = Head.IndexToLocFormat;
            Result.UnitsPerEm = Head.UnitsPerEm;
            Result.MinX = Head.MinX;
            Result.MaxX = Head.MaxX;
            Result.MinY = Head.MinY;
            Result.MaxY = Head.MaxY;
        }
        else if (TagEquals(TableRecord.Tag, "maxp")) {
            maxp_table MaxP = ParseTTFMaxProfileTable(FilePointer + TableRecord.Offset);
            Assert(MaxP.Version == 0x00010000);
            nGlyphs = MaxP.NumGlyphs;
            Result.nGlyphs = MaxP.NumGlyphs;
        }
        else if (TagEquals(TableRecord.Tag, "loca")) {
            Result.LocaOffset = TableRecord.Offset;
        }
        else if (TagEquals(TableRecord.Tag, "glyf")) {
            Result.GlyfOffset = TableRecord.Offset;
        }
        else if (TagEquals(TableRecord.Tag, "hhea")) {
            hhead_table HHeadTable = ParseHorizontalHeadTable(FilePointer + TableRecord.Offset);
            nHMetrics = HHeadTable.NumberOfHMetrics;
            HheaLineJump = HHeadTable.Ascender - HHeadTable.Descender + HHeadTable.LineGap;
        }
        else if (TagEquals(TableRecord.Tag, "hmtx")) {
            HorizontalMetricsTable = (long_hor_metric*)(FilePointer + TableRecord.Offset);
        }
        else if (TagEquals(TableRecord.Tag, "vhea")) {
            vhead_table VHeadTable = ParseVerticalHeadTable(FilePointer + TableRecord.Offset);
            nVMetrics = VHeadTable.NumOfLongVerMetrics;
        }
        else if (TagEquals(TableRecord.Tag, "vmtx")) {
            VerticalMetricsTable = (vertical_metric*)(FilePointer + TableRecord.Offset);
        }
        else if (TagEquals(TableRecord.Tag, "OS/2")) {
            os2_table OS2 = ParseTTFOS2Table(FilePointer + TableRecord.Offset);
            fsSelection_flags Flags = (fsSelection_flags)OS2.fsSelection;
            if (Flags & FS_USE_TYPO_METRICS) {
                Result.LineJump = OS2.sTypoAscender - OS2.sTypoDescender + OS2.sTypoLineGap;
            }
        }
        else if (TagEquals(TableRecord.Tag, "cmap")) {
            uint8* CMapPointer = FilePointer + TableRecord.Offset;
            cmap_header CMapHeader = ParseTTFCMapHeader(CMapPointer);

            uint8* EncodingsPointer = CMapPointer + sizeof(cmap_header);
            for (int j = 0; j < CMapHeader.NumTables; j++) {
                encoding_record Encoding = ParseTTFEncoding(EncodingsPointer);

                if (Encoding.PlatformID == 3 && Encoding.EncodingID == 1) {
                    cmap_subtable Subtable = ParseTTFCmapSubtable(CMapPointer + Encoding.Offset);
                    Assert(Subtable.Format == 4);
                    SegCount = Subtable.SegCountX2 >> 1;

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
        Result.LocaOffset == 0 ||
        Result.GlyfOffset == 0 || 
        nHMetrics == 0 || 
        HorizontalMetricsTable == 0
    ) {
        Raise("Some font table wasn't found");
    }

    if (Result.LineJump == 0) {
        Result.LineJump = HheaLineJump;
    }

    FWORD* OtherLeftSideBearings = (FWORD*)(HorizontalMetricsTable + nHMetrics);
    FWORD* OtherTopSideBearings = 0;
    if (nVMetrics != 0 && VerticalMetricsTable != 0) {
        OtherTopSideBearings = (FWORD*)(VerticalMetricsTable + nVMetrics);
    }

    // Locations of glyphs in file
    uint32* GlyphOffsets = new uint32[nGlyphs+1];
    uint32* LocationsTable = (uint32*)(FilePointer + Result.LocaOffset);
    FillGlyphOffsets(GlyphOffsets, LocationsTable, IndexToLocFormat, nGlyphs);

    // Getting number of glyph contours and points (to compute size)
    uint32 TotalPoints = 0;
    uint32 TotalOnCurvePoints = 0;
    uint32 TotalContours = 0;
    uint32 TotalCompositeRecords = 0;
    uint8* GlyfTable = FilePointer + Result.GlyfOffset;
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
                Log(log_level::Error, ErrorBuffer);
            }
            else {
                GlyphID += BigEndian(IDDeltas[i]);
            }
        }
        else {
            GlyphID = c + BigEndian(IDDeltas[i]);
        }

        Assert(GlyphID > 0 && GlyphID < nGlyphs);
        if (c != ' ') Result.GlyphIDs[c - '!'] = GlyphID;

        uint32 Offset = GlyphOffsets[GlyphID];
        Assert(Offset < FileInfo.Size);
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

                Result.nChildren[c - '!'] = nChildren;
            }
            
            // Simple glyphs
            if (GlyphHeader.NumberOfContours > 0) {
                TotalContours += GlyphHeader.NumberOfContours;
                Result.Contours[c - '!'].resize(GlyphHeader.NumberOfContours);

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
                    glyph_contour* Contour = &Result.Contours[c - '!'][k];
                    Contour->nPoints = nPoints;
                    Contour->Endpoint = CharacterPoints - 1;
                }

                Result.nPoints[c - '!'] = CharacterPoints;
                Result.nOnCurvePoints[c - '!'] = CharacterOnCurvePoints;
                TotalPoints += CharacterPoints;
                TotalOnCurvePoints += CharacterOnCurvePoints;
            }

            if (c != ' ') Result.GlyphOffsets[c - '!'] = Offset;
        }
        else if (c != ' ') Raise("Current glyph has length 0.");
        
        if (c == ' ') {
            Result.SpaceAdvance = BigEndian(HorizontalMetricsTable[GlyphID].AdvanceWidth);
        }
        else {
            // Horizontal metrics
            if (GlyphID < nHMetrics) {
                Result.AdvanceWidths[c - '!']    = BigEndian(HorizontalMetricsTable[GlyphID].AdvanceWidth);
                Result.LeftSideBearings[c - '!'] = BigEndian(HorizontalMetricsTable[GlyphID].LeftSideBearing);
            }
            else {
                Result.AdvanceWidths[c - '!']    = BigEndian(HorizontalMetricsTable[nHMetrics-1].AdvanceWidth);
                Result.LeftSideBearings[c - '!'] = BigEndian(OtherLeftSideBearings[GlyphID - nHMetrics]);
            }

            // Vertical metrics
            if (VerticalMetricsTable) {
                if (GlyphID < nVMetrics) {
                    Result.AdvanceHeights[c - '!']  = BigEndian(VerticalMetricsTable[GlyphID].AdvanceHeight);
                    Result.TopSideBearings[c - '!'] = BigEndian(VerticalMetricsTable[GlyphID].TopSideBearing);
                }
                else {
                    Result.AdvanceHeights[c - '!'] = BigEndian(VerticalMetricsTable[nVMetrics - 1].AdvanceHeight);
                    Result.TopSideBearings[c - '!'] = BigEndian(OtherTopSideBearings[GlyphID - nHMetrics]);
                }
            }
        }
    }

    delete [] GlyphOffsets;

    Result.nOnCurve = TotalOnCurvePoints;

    Result.Size = TotalContours * sizeof(glyph_contour);
    Result.Size += TotalPoints * sizeof(glyph_contour_point);
    Result.Size += TotalCompositeRecords * sizeof(composite_glyph_record);
    Result.nTotalPoints = TotalPoints;

    return Result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------+
// | Font loading                                                                                                                             |
// +------------------------------------------------------------------------------------------------------------------------------------------+
game_font LoadFont(memory_arena* Permanent, memory_arena* Transient, preprocessed_font* Font) {
    using namespace ttf;

    game_font Result = {};
    Result.SpaceAdvance = Font->SpaceAdvance;
    Result.UnitsPerEm = Font->UnitsPerEm;
    Result.LineJump = Font->LineJump;
    Result.nOnCurve = Font->nOnCurve;
    Result.nPoints = Font->nTotalPoints;
    Result.MinX = Font->MinX;
    Result.MaxX = Font->MaxX;
    Result.MinY = Font->MinY;
    Result.MaxY = Font->MaxY;

    uint8* FilePointer = (uint8*)Font->FileContent;

    uint32* GlyphOffsets = PushArray(Transient, Font->nGlyphs+1, uint32);
    uint32* LocationsTable = (uint32*)(FilePointer + Font->LocaOffset);
    FillGlyphOffsets(GlyphOffsets, LocationsTable, Font->IndexToLocFormat, Font->nGlyphs);

    // Getting glyph data
    uint8* GlyfTable = FilePointer + Font->GlyfOffset;
    for (char c = '!'; c <= '~'; c++) {
        matrix2 Transform = Identity2;
        v2 Translation = V2(0,0);

        uint32 Offset = Font->GlyphOffsets[c - '!'];
        uint8* GlyphData = GlyfTable + Offset;
        glyph_header GlyphHeader = ParseTTFGlyphHeader(GlyphData);
        GlyphData += sizeof(glyph_header);

        game_font_character* Character = &Result.Characters[c - '!'];
        Character->Letter   = c;
        Character->Width    = Font->AdvanceWidths[c - '!'];
        Character->Height   = GlyphHeader.MaxY - GlyphHeader.MinY;
        Character->Left     = Font->LeftSideBearings[c - '!'];
        Character->Top      = GlyphHeader.MaxY;
        Character->nPoints  = Font->nPoints[c - '!'];
        Character->nOnCurve = Font->nOnCurvePoints[c - '!'];

        // Composite glyphs
        if (GlyphHeader.NumberOfContours < 0) {
            uint32 nChildren = Font->nChildren[c - '!'];
            uint16* Pointer = (uint16*)GlyphData;
            composite_glyph_record Record = {};

            if (nChildren == 0) Raise("Composite glyph with no children.");
            else if (nChildren > 1) {
                Character->nChildren = nChildren;
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
                    Transform.XX = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                    Transform.YY = Transform.XX;
                }
                else if (Flags & WE_HAVE_AN_X_AND_Y_SCALE) {
                    Transform.XX = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                    Transform.YY = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                }
                else if (Flags & WE_HAVE_A_TWO_BY_TWO) {
                    Transform.XX = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                    Transform.YX = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                    Transform.XY = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                    Transform.YY = GetF2DOT14(BigEndian(*(int16*)Pointer++));
                }

                if (Flags & ARGS_ARE_XY_VALUES) {
                    if (Flags & ROUND_XY_TO_GRID) {
                        // TODO: Round to pixel grid
                        Raise("Not implemented");
                    }

                    if (Flags & SCALED_COMPONENT_OFFSET) {
                        v2 ComponentOffset = V2(X, Y);
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
                    composite_glyph_record* Record = PushStruct(Permanent, composite_glyph_record);
                    char Found = 0;
                    for (char Child = '!'; Child <= '~'; Child++) {
                        if (Font->GlyphIDs[Child - '!'] == ChildGlyphIndex) {
                            Found = Child;
                            break;
                        }
                    }
                    if (Found == 0) Raise("Child glyph was not one of the loaded glyphs");
                    Record->Child = Found;
                    Record->X = Translation.X;
                    Record->Y = Translation.Y;
                    Record->Transform = Transform;
                }
            } while(Flags & MORE_COMPONENTS);
        }

        // Sets the subglyph number of contours if glyph is composite with only one children
        Character->nContours = GlyphHeader.NumberOfContours;

        // Simple glyphs
        if (GlyphHeader.NumberOfContours > 0) {
            uint16* EndPtsOfContours = (uint16*)GlyphData;
            Character->Contours = PushArray(Permanent, Character->nContours, glyph_contour);
            
            for (int i = 0; i < GlyphHeader.NumberOfContours; i++) {
                Character->Contours[i] = Font->Contours[c - '!'][i];
            }
    
            uint16 nPoints = BigEndian(EndPtsOfContours[GlyphHeader.NumberOfContours - 1]) + 1;
            Character->Data = Permanent->Base + Permanent->Used;
    
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
            int FontPointIndex = 0;
            uint8* MemoryLayoutStart = Permanent->Base + Permanent->Used;
            int TotalPoints = 0;
            for (int i = 0; i < GlyphHeader.NumberOfContours; i++) {
                glyph_contour* Contour = &Character->Contours[i];
                Contour->Points = (glyph_contour_point*)(Permanent->Base + Permanent->Used);

                simple_glyph_flag Flag = (simple_glyph_flag)*pFlag;
                Assert(Flag & ON_CURVE_POINT);
                bool PreviousOnCurve = false;
                Contour->Endpoint = BigEndian(EndPtsOfContours[i]);

                bool Start = true;
                uint32 OutPointIndex = 0;
                linked_list Vertices = {};

                for (; FontPointIndex <= Contour->Endpoint; FontPointIndex++) {
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
                        glyph_contour_point* Result = PushArray(Permanent, 2, glyph_contour_point);
                        Result[0] = {
                            OutPointIndex++,
                            MiddleX, MiddleY,
                            true,
                        };
                        link* Link = PushStruct(Transient, link);
                        Link->Data = Result;
                        Vertices.PushBack(Link);

                        Result[1] = {
                            OutPointIndex++,
                            (float)X, (float)Y,
                            false
                        };

                        TotalPoints += 2;
                    }
                    else {
                        glyph_contour_point* Result = PushStruct(Permanent, glyph_contour_point);
                        Result[0] = {
                            OutPointIndex++,
                            (float)X, (float)Y,
                            OnCurve
                        };

                        if (OnCurve) {
                            link* Link = PushStruct(Transient, link);
                            Link->Data = Result;
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
                Contour->IsConvex   = IsConvex(Polygon);
                Contour->IsExterior = GetArea(Polygon) > 0;
            }
            Assert(TotalPoints == Font->nPoints[c - '!']);
            Assert(Permanent->Base + Permanent->Used == MemoryLayoutStart + sizeof(glyph_contour_point) * Font->nPoints[c - '!']);
        }
    }

    return Result;
}

uint32 WriteFontVertices(memory_arena* Arena, game_font* Font) {
    uint32 Index = 0;
    Font->Vertices = (float*)(Arena->Base + Arena->Used);
    for (char c = '!'; c <= '~'; c++) {
        game_font_character* Character = &Font->Characters[c - '!'];
        Character->VertexOffset = Index;

        for (int i = 0; i < Character->nContours; i++) {
            glyph_contour Contour = Character->Contours[i];
            glyph_contour_point Last = Contour.Points[Contour.nPoints-1];
            for (int j = 0; j < Contour.nPoints; j++) {
                glyph_contour_point* First = &Contour.Points[j];
                if (First->OnCurve) {
                    First->Index = Index;
                    glyph_contour_point Second = Contour.Points[(j + 1) % Contour.nPoints];
                    glyph_contour_point Third = {};
                    if (Second.OnCurve) {
                        Third = Second;
                        Second.X = 0.5f * (First->X + Third.X);
                        Second.Y = 0.5f * (First->Y + Third.Y);
                    }
                    else {
                        Contour.Points[(j + 1) % Contour.nPoints].Index = Index + 1;
                        Third = Contour.Points[(j + 2) % Contour.nPoints];
                    }

                    float* Out = PushArray(Arena, 12, float);
                    Out[0]  = First->X - 0.5f*Character->Left;
                    Out[1]  = -First->Y;
                    Out[2]  = 0.0f;
                    Out[3]  = 0.0f;
                    Out[4]  = Second.X - 0.5f*Character->Left;
                    Out[5]  = -Second.Y;
                    Out[6]  = 1.0f;
                    Out[7]  = 0.0f;
                    Out[8]  = Third.X - 0.5f*Character->Left;
                    Out[9]  = -Third.Y;
                    Out[10] = 0.0f;
                    Out[11] = 1.0f;

                    Index += 3;
                }
            }
        }
    }

    Assert(Index == 3 * Font->nOnCurve);

    return Index;
}

void WriteFontCurveTriangles(
    memory_arena* Arena,
    game_font* Font
) {
    xarray<uint32> InteriorTriangles;
    xarray<uint32> ExteriorTriangles;

    Font->Elements = (uint32*)(Arena->Base + Arena->Used);
    uint32 Offset = 0;
    Font->nCurveTriangles = 0;
    for (char c = '!'; c <= '~'; c++) {
        game_font_character* Character = &Font->Characters[c - '!'];
        Character->nInteriorCurves = 0;
        Character->nExteriorCurves = 0;

        for (int i = 0; i < Character->nContours; i++) {
            glyph_contour Contour = Character->Contours[i];
            
            for (int j = 0; j < Contour.nPoints; j++) {
                glyph_contour_point* Point = &Contour.Points[j];
                if (!Point->OnCurve) {
                    glyph_contour_point* Previous = &Contour.Points[j-1];
                    glyph_contour_point* Next = &Contour.Points[(j+1) % Contour.nPoints];

                    v2 A = V2(Previous->X, Previous->Y);
                    v2 B = V2(Point->X, Point->Y);
                    v2 C = V2(Next->X, Next->Y);
                    triangle2 T = { A, B, C };
                    float Area = GetArea(T);

                    if (Area < 0) {
                        InteriorTriangles.Insert(Point->Index-1);
                        InteriorTriangles.Insert(Point->Index);
                        InteriorTriangles.Insert(Point->Index+1);
                        Character->nInteriorCurves += 1;
                    }
                    else if (Area > 0) {
                        ExteriorTriangles.Insert(Point->Index-1);
                        ExteriorTriangles.Insert(Point->Index);
                        ExteriorTriangles.Insert(Point->Index+1);
                        Character->nExteriorCurves += 1;
                    }
                }
            }
        }

        Character->InteriorCurvesOffset = Offset;
        if (Character->nInteriorCurves > 0) {
            Font->nCurveTriangles += Character->nInteriorCurves;
            uint32* Elements = PushArray(Arena, 3 * Character->nInteriorCurves, uint32);
            for (int i = 0; i < 3 * Character->nInteriorCurves; i++) {
                uint32 Element = InteriorTriangles[i];
                Elements[i] = Element;
            }
            Offset += 3 * Character->nInteriorCurves;
        }

        Character->ExteriorCurvesOffset = Offset;
        if (Character->nExteriorCurves > 0) {
            Font->nCurveTriangles += Character->nExteriorCurves;
            uint32* Elements = PushArray(Arena, 3 * Character->nExteriorCurves, uint32);
            for (int i = 0; i < 3 * Character->nExteriorCurves; i++) {
                uint32 Element = ExteriorTriangles[i];
                Elements[i] = Element;
            }
            Offset += 3 * Character->nExteriorCurves;
        }

        InteriorTriangles.Clear();
        ExteriorTriangles.Clear();
    }
}

void WriteFontSolidTriangles(memory_arena* Permanent, memory_arena* Transient, game_font* Font) {
    using namespace ttf;

    uint32 Offset = 3 * Font->nCurveTriangles;
    Font->nSolidTriangles = 0;
    for (char c = '!'; c <= '~'; c++) {
        game_font_character* Character = &Font->Characters[c - '!'];
        Character->SolidTrianglesOffset = Offset;
        Character->nSolidTriangles = 0;

        for (int i = 0; i < Character->nContours; i++) {
            glyph_contour Contour = Character->Contours[i];
            
            if (Contour.IsExterior) {
                xarray<triangle2> VoidTriangles;
                glyph_polygon Polygon = {};

                // Add points in contour to the polygon
                for (int j = 0; j < Contour.nPoints; j++) {
                    glyph_contour_point* Point = &Contour.Points[j];
                    glyph_contour_point* Previous = &Contour.Points[(j+Contour.nPoints-1) % Contour.nPoints];
                    if (!Previous->OnCurve) {
                        Previous = &Contour.Points[(j+Contour.nPoints-2) % Contour.nPoints];
                    }
                    glyph_contour_point* Next = &Contour.Points[(j+1) % Contour.nPoints];
                    if (!Next->OnCurve) {
                        Next = &Contour.Points[(j+2) % Contour.nPoints];
                    }

                    v2 A = GetContourPointV2(Previous);
                    v2 B = GetContourPointV2(Point);
                    v2 C = GetContourPointV2(Next);
                    triangle2 T = { A, B, C };
                    float Area = GetArea(T);

                    if (Area < 0) {
                        VoidTriangles.Insert(T);
                    } else if (!Point->OnCurve) continue;

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
                for (int j = 0; j < Character->nContours; j++) {
                    glyph_contour InteriorContour = Character->Contours[j];

                    if (!InteriorContour.IsExterior) {
                        v2 TestPoint = V2(InteriorContour.Points[0].X, InteriorContour.Points[0].Y);
                        if (IsInside(Polygon, TestPoint)) {
                            glyph_polygon NewInteriorPolygon = {};
                            for (int k = 0; k < InteriorContour.nPoints; k++) {
                                link* Link = PushStruct(Transient, link);
                                Link->Data = &InteriorContour.Points[k];
                                NewInteriorPolygon.Vertices.PushBack(Link);
                                v2 A = GetContourPointV2(&InteriorContour.Points[(k+InteriorContour.nPoints-1)%InteriorContour.nPoints]);
                                v2 B = GetContourPointV2(&InteriorContour.Points[k]);
                                v2 C = GetContourPointV2(&InteriorContour.Points[(k+1)%InteriorContour.nPoints]);
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

                            nInteriorPoints += InteriorContour.nPoints;
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
                    glyph_contour_point First = {}, Second = {}, Third = {};

                    link* Vertex = Polygon.Vertices.First;

                    bool Found = false;
                    bool EmptyTriangle = false;
                    do {
                        First  = *(glyph_contour_point*)Vertex->Previous->Data;
                        Second = *(glyph_contour_point*)Vertex->Data;
                        Third  = *(glyph_contour_point*)Vertex->Next->Data;

                        v2 A = V2(First.X, First.Y);
                        v2 B = V2(Second.X, Second.Y);
                        v2 C = V2(Third.X, Third.Y);

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
                                    glyph_contour_point* ContourPoint = (glyph_contour_point*)TestVertex->Data;
                                    if (
                                        ContourPoint->Index != First.Index && 
                                        ContourPoint->Index != Second.Index && 
                                        ContourPoint->Index != Third.Index
                                    ) {
                                        v2 P = GetContourPointV2(ContourPoint);
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
                            First  = *(glyph_contour_point*)Vertex->Previous->Data;
                            Second = *(glyph_contour_point*)Vertex->Data;
                            Third  = *(glyph_contour_point*)Vertex->Next->Data;

                            v2 A = V2(First.X, First.Y);
                            v2 B = V2(Second.X, Second.Y);
                            v2 C = V2(Third.X, Third.Y);

                            triangle2 T = { A, B, C };
                            float Area = GetArea(T);

                            if (Area > 0) {
                                // Ignore intersections with void triangles if it is the last one
                                uint32* Out = PushArray(Permanent, 3, uint32);
                                Out[0] = First.Index;
                                Out[1] = Second.Index;
                                Out[2] = Third.Index;
            
                                Character->nSolidTriangles++;
    
                                Offset += 3;
                            }
                            break;
                        }
                    }

                    if (!EmptyTriangle) {
                        uint32* Out = PushArray(Permanent, 3, uint32);
                        Out[0] = First.Index;
                        Out[1] = Second.Index;
                        Out[2] = Third.Index;
    
                        Character->nSolidTriangles++;
    
                        Offset += 3;
                    }
                
                    Polygon.Vertices.Break(Vertex);
                    VertexCount--;
                }

                Font->nSolidTriangles += Character->nSolidTriangles;

                VoidTriangles.Clear();
            }
        }
    }
}

void TriangulateFont(memory_arena* FontsArena, memory_arena* Transient, game_font* Font) {
    WriteFontVertices(FontsArena, Font);
    WriteFontCurveTriangles(FontsArena, Font);
    WriteFontSolidTriangles(FontsArena, Transient, Font);
}

#endif