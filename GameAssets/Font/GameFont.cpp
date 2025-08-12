#include "GameFont.h"
#include "Win32PlatformLayer.h"

// +------------------------------------------------------------------------------------------------------------------------------------------+
// | Utilities for TTF file parsing                                                                                                           |
// +------------------------------------------------------------------------------------------------------------------------------------------+

void FillGlyphOffsets(uint32* GlyphOffsets, uint32* LocationsTable, int16 IndexToLocFormat, uint16 nGlyphs) {
    switch(IndexToLocFormat) {
        case 0: {
            for (int i = 0; i <= nGlyphs; i++) {
                GlyphOffsets[i] = (uint32)BigEndian(LocationsTable[i]) << 1;
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

// void LoadFTBMP(FT_Bitmap* SourceBMP, game_bitmap* DestBMP) {
//     DestBMP->Header.Width = SourceBMP->width;
//     DestBMP->Header.Height = SourceBMP->rows;
//     uint32* DestRow = DestBMP->Content + DestBMP->Header.Width * (DestBMP->Header.Height - 1);
//     uint8* Source = SourceBMP->buffer;
//     for (int Y = 0; Y < SourceBMP->rows; Y++) {
//         uint32* Pixel = DestRow;
//         for (int X = 0; X < SourceBMP->width; X++) {
//             // FreeType BMPs come with only one byte representing alpha. We load it as a white BMP so changing
//             // the color is easier with OpenGL.
//             *Pixel++ = (*Source++ << 24) | 0x00ffffff;
//         }
//         DestRow -= SourceBMP->pitch;
//     }
// }

// void GetFontBMPWidthAndHeight(FT_Face Font, uint32* Width, uint32* Height) {
//     uint32 ResultWidth = 0, ResultHeight = 0, RowWidth = 0, MaxHeight = 0;
//     FT_Error FTError = FT_Set_Char_Size(Font, 0, LOAD_POINTS*64, 128, 128);
//     if (FTError) Assert(false);

//     char Starts[3] = {'!', 'A', 'a'};
//     char Ends[3] = {'@', '`', '~'};

//     for (int i = 0; i < 3; i++) {
//         for (unsigned char c = Starts[i]; c <= Ends[i]; c++) {
//             FTError = FT_Load_Char(Font, c, FT_LOAD_RENDER);
//             if (FTError) Assert(false);

//             FT_GlyphSlot Slot = Font->glyph;
//             FT_Bitmap FTBMP = Slot->bitmap;

//             RowWidth += FTBMP.width;
//             if (MaxHeight < FTBMP.rows) MaxHeight = FTBMP.rows;
//         }

//         if (RowWidth > ResultWidth) ResultWidth = RowWidth;
//         RowWidth = 0;

//         ResultHeight += MaxHeight;
//         MaxHeight = 0;
//     }

//     *Width = ResultWidth;
//     *Height = ResultHeight;
// }

// uint64 ComputeNeededMemoryForFont(const char* Path) {
//     FT_Library FTLibrary;
//     FT_Face Font;
//     FT_Error error = FT_Init_FreeType(&FTLibrary);
//     if (error) Assert(false);

//     error = FT_New_Face(FTLibrary, Path, 0, &Font);
//     if (error == FT_Err_Unknown_File_Format) Raise("Freetype error: Unknown file format.");
//     else if (error) Assert(false);
    
//     uint32 Width = 0, Height = 0;
//     GetFontBMPWidthAndHeight(Font, &Width, &Height);

//     FT_Done_Face(Font);
//     FT_Done_FreeType(FTLibrary);

//     return 4 * Width * Height;
// }

// +------------------------------------------------------------------------------------------------------------------------------------------+
// | Font preprocessing                                                                                                                       |
// +------------------------------------------------------------------------------------------------------------------------------------------+

preprocessed_font PreprocessFont(read_file_result File) {
    using namespace ttf;

    preprocessed_font Result = {};
    Result.File = File;

    uint8* FilePointer = (uint8*)File.Content;

    font_header Header = ParseTTFHeader(FilePointer);
    Assert(Header.SFNTVersion == 0x00010000);

    long_hor_metric* HorizontalMetricsTable = 0;
    vertical_metric* VerticalMetricsTable = 0;
    int16 IndexToLocFormat = 0;
    uint16 nHMetrics = 0, nVMetrics = 0;

    // CMap pointers
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
            Assert(OS2.version >= 2);
            Result.LineJump = OS2.sTypoAscender - OS2.sTypoDescender + OS2.sTypoLineGap;
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
                sprintf_s(ErrorBuffer, "Character '%c' wasn't found in font.", c);
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
                sprintf_s(ErrorBuffer, "Character '%c' wasn't found in font.", c);
                Log(Error, ErrorBuffer);
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
                TotalPoints += CharacterPoints;
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

    Result.Size = TotalContours * sizeof(glyph_contour);
    // Each point is 2 floats + 1 bool (on-off curve)
    Result.Size += TotalPoints * (2 * sizeof(float) + sizeof(bool));
    Result.Size += TotalCompositeRecords * sizeof(composite_glyph_record);

    return Result;
}

// +------------------------------------------------------------------------------------------------------------------------------------------+
// | Font loading                                                                                                                             |
// +------------------------------------------------------------------------------------------------------------------------------------------+

game_font LoadFont(memory_arena* Arena, preprocessed_font* Font) {
    using namespace ttf;

    game_font Result = {};
    Result.SpaceAdvance = Font->SpaceAdvance;
    Result.UnitsPerEm = Font->UnitsPerEm;
    Result.LineJump = Font->LineJump;

    uint8* FilePointer = (uint8*)Font->File.Content;

    uint32* GlyphOffsets = new uint32[Font->nGlyphs+1];
    uint32* LocationsTable = (uint32*)(FilePointer + Font->LocaOffset);
    FillGlyphOffsets(GlyphOffsets, LocationsTable, Font->IndexToLocFormat, Font->nGlyphs);

    memory_arena TempArena = AllocateMemoryArena(Kilobytes(32));

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
        Character->Letter    = c;
        Character->Width     = Font->AdvanceWidths[c - '!'];
        Character->Height    = Font->AdvanceHeights[c - '!'];
        Character->Left      = Font->LeftSideBearings[c - '!'];
        Character->Top       = Font->TopSideBearings[c - '!'];
        Character->nPoints   = Font->nPoints[c - '!'];

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
                    // if (Flags & ROUND_XY_TO_GRID) {
                    //     // TODO: Round to pixel grid
                    //     Raise("Not implemented");
                    // }

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

                // if (Flags & USE_MY_METRICS) {
                //     // TODO: Use the metrics of this component
                //     Raise("Not implemented");
                // }

                if (nChildren == 1) {
                    GlyphData = GlyfTable + GlyphOffsets[ChildGlyphIndex];
                    GlyphHeader = ParseTTFGlyphHeader(GlyphData);
                    GlyphData += sizeof(glyph_header);
                }
                else {
                    composite_glyph_record* Record = PushStruct(Arena, composite_glyph_record);
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
            Character->Contours = PushArray(Arena, Character->nContours, glyph_contour);
            PushArray(Arena, Character->nPoints, bool);
            
            uint8* Pointer = (uint8*)Character->Contours;
            xarray<glyph_contour*> Contours = {};
            for (int i = 0; i < GlyphHeader.NumberOfContours; i++) {
                glyph_contour* ContourStart = (glyph_contour*)Pointer;
                *ContourStart = Font->Contours[c - '!'][i];
                Contours.Insert(ContourStart);
                Pointer += sizeof(glyph_contour) + ContourStart->nPoints * sizeof(bool);
            }
    
            uint16 nPoints = BigEndian(EndPtsOfContours[GlyphHeader.NumberOfContours - 1]) + 1;
            Character->Data = Arena->Base + Arena->Used;
    
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
            uint8* MemoryLayoutStart = Arena->Base + Arena->Used;
            int nFloats = 0;
            for (int i = 0; i < GlyphHeader.NumberOfContours; i++) {
                simple_glyph_flag Flag = (simple_glyph_flag)*pFlag;
                Assert(Flag & ON_CURVE_POINT);
                bool PreviousOnCurve = false;
                int Endpoint = BigEndian(EndPtsOfContours[i]);

                bool Start = true;
                int OutPointIndex = 0;
                linked_list OnCurvePoints = {};

                for (; FontPointIndex <= Endpoint; FontPointIndex++) {
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
                        float* Result = PushArray(Arena, 4, float);
                        Result[0] = MiddleX;
                        Result[1] = MiddleY;
                        link* Link = PushStruct(&TempArena, link);
                        Link->Data = Result;
                        OnCurvePoints.PushBack(Link);

                        Contours[i]->IsOnCurve[OutPointIndex++] = true;
                        Result[2] = X;
                        Result[3] = Y;
                        Contours[i]->IsOnCurve[OutPointIndex++] = false;

                        nFloats += 4;
                    }
                    else {
                        float* Result = PushArray(Arena, 2, float);
                        Result[0] = X;
                        Result[1] = Y;
                        Contours[i]->IsOnCurve[OutPointIndex++] = OnCurve;

                        if (OnCurve) {
                            link* Link = PushStruct(&TempArena, link);
                            Link->Data = Result;
                            OnCurvePoints.PushBack(Link);
                        }

                        nFloats += 2;
                    }
                    
                    PreviousOnCurve = OnCurve;
                    LastX = X;
                    LastY = Y;
                    NextTTFGlyphFlag(pFlag);
                }
                OnCurvePoints.CloseCircle();

                // Post processing
                polygon Polygon = { OnCurvePoints };
                Contours[i]->IsConvex   = IsConvex(Polygon);
                Contours[i]->IsExterior = Area(Polygon) > 0;

                ClearArena(&TempArena);
            }
            Assert(nFloats == 2 * Font->nPoints[c - '!']);
            Assert(Arena->Base + Arena->Used == MemoryLayoutStart + 2 * sizeof(float) * Font->nPoints[c - '!']);
        }
    }

    FreeMemoryArena(&TempArena);
    
    // FT_Library FTLibrary;
    // FT_Face Font;
    // FT_Error error = FT_Init_FreeType(&FTLibrary);
    // if (error) Assert(false);

    // error = FT_New_Face(FTLibrary, Asset->File.Path, 0, &Font);
    // if (error == FT_Err_Unknown_File_Format) Raise("Freetype error: Unknown file format.");
    // else if (error) Assert(false);
        
    // // Initializing char bitmaps
    // error = FT_Set_Char_Size(Font, 0, LOAD_POINTS * 64, 128, 128);
    // if (error) Assert(false);

    // error = FT_Load_Char(Font, ' ', FT_LOAD_RENDER);
    // if (error) Assert(false);
    // Result.SpaceAdvance = Font->glyph->advance.x >> 6;

    // uint32 Width = 0, Height = 0;
    // GetFontBMPWidthAndHeight(Font, &Width, &Height);

    // Result.Bitmap = MakeEmptyBitmap(Arena, Width, Height, true);

    // uint32* Buffer = new uint32[3686400];
    // memory_arena BufferArena = MemoryArena(3686400, (uint8*)Buffer);

    // unsigned char c = '!';
    // int X = 0;
    // int Y = 0;
    // int MaxHeight = 0;
    // for (int i = 0; i < FONT_CHARACTERS_COUNT; i++) {
    //     game_font_character* pCharacter = &Result.Characters[i];
    //     error = FT_Load_Char(Font, c, FT_LOAD_RENDER);
    //     if (error) Assert(false);
        
    //     FT_GlyphSlot Slot = Font->glyph;
    //     FT_Bitmap FTBMP = Slot->bitmap;
    //     game_bitmap Test = MakeEmptyBitmap(&BufferArena, FTBMP.width, FTBMP.rows, true);
    //     LoadFTBMP(&FTBMP, &Test);

    //     pCharacter->Letter = c;
    //     pCharacter->Advance = Slot->advance.x >> 6;
    //     pCharacter->Left = Slot->bitmap_left;
    //     pCharacter->Top = Slot->bitmap_top;
    //     pCharacter->Height = FTBMP.rows;
    //     pCharacter->Width = FTBMP.width;

    //     for (int Row = 0; Row <= FTBMP.rows; Row++) {
    //         for (int Col = 0; Col <= FTBMP.width; Col++) {
    //             uint32* PixelAddress = GetPixelAddress(&Result.Bitmap, X + Col, Y + Row);
    //             *PixelAddress = GetPixel(&Test, Col, Row);
    //         }
    //     }

    //     pCharacter->AtlasX = X;
    //     pCharacter->AtlasY = Y;

    //     PopSize(&BufferArena, FTBMP.width * FTBMP.rows * 4);

    //     if (FTBMP.rows > MaxHeight) MaxHeight = FTBMP.rows;
    //     X += FTBMP.width;
    //     if (c == '@' || c == '`') {
    //         X = 0;
    //         Y += MaxHeight;
    //         MaxHeight = 0;
    //     }

    //     c++;
    //}

    // Result.LineJump = Result.Characters[0].Height * 3 / 2;

    // FT_Done_Face(Font);
    // FT_Done_FreeType(FTLibrary);

    // delete [] Buffer;

    // delete [] GlyphOffsets;

    return Result;
}