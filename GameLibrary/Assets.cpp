#include "pch.h"
#include "Assets.h"

/*
    TODO:
        - Load .obj file directly.
        - Asset hot loading.
*/


// Asset loading
 // BMP
void ClearBitmap(loaded_bmp* Bitmap) {
    if (Bitmap->Content) {
        int32 TotalBitmapSize = Bitmap->Header.Width * Bitmap->Header.Height * 8;
        ZeroSize(TotalBitmapSize, Bitmap->Content);
    }
}

loaded_bmp MakeEmptyBitmap(memory_arena* Arena, int32 Width, int32 Height, bool ClearToZero = true) {
    loaded_bmp Result = { 0 };
    Result.Header = { 0 };
    Result.Header.Width = Width;
    Result.Header.Height = Height;
    Result.Header.BitsPerPixel = 32;
    Result.BytesPerPixel = 4;
    Result.Pitch = 4 * Width;
    int32 TotalBitmapSize = Width * Height * 32;
    Result.Header.FileSize = TotalBitmapSize;

    Result.Header.RedMask = 0x00ff0000;
    Result.Header.GreenMask = 0x0000ff00;
    Result.Header.BlueMask = 0x000000ff;
    Result.AlphaMask = 0xff000000;

    Result.Content = (uint32*)PushSize(Arena, TotalBitmapSize / 8);
    if (ClearToZero) {
        ClearBitmap(&Result);
    }
    return Result;
}

loaded_bmp LoadBMP(platform_read_entire_file* PlatformReadEntireFile, const char* Path) {
    loaded_bmp Result = { 0 };
    Result.Handle = 0;
    read_file_result ReadResult = PlatformReadEntireFile(Path);
    if (ReadResult.ContentSize != 0) {
        bitmap_header* Header = (bitmap_header*)ReadResult.Content;
        Result.Header = *Header;
        uint32 BytesPerPixel = Header->BitsPerPixel >> 3;
        Result.BytesPerPixel = BytesPerPixel;
        Result.Pitch = Header->Width * BytesPerPixel;
        Result.Content = (uint32*)((uint8*)ReadResult.Content + Header->BitmapOffset);

        bool HasAlpha = false;
        if (Result.Header.BitsPerPixel == 32 && Result.Header.Compression == 3) {
            uint32 AlphaMask = ~(Result.Header.RedMask | Result.Header.GreenMask | Result.Header.BlueMask);
            // If not all Alphas are zero, we need to use them
            uint32* Contents = Result.Content;
            for (int32 i = 0; i < Result.Header.Height * Result.Header.Width; i++) {
                if ((*Contents++ & AlphaMask) > 0) {
                    HasAlpha = true;
                    break;
                }
            }

            // If all alphas are zero, turn them to one
            Contents = Result.Content;
            if (!HasAlpha) {
                for (int32 j = 0; j < Result.Header.Height * Result.Header.Width; j++) {
                    *Contents = AlphaMask | (*Contents++ & ~AlphaMask);
                }
            }
        }
    }
    return Result;
}

// WAV
game_sound LoadWAV(platform_read_entire_file* PlatformReadEntireFile, const char* FileName) {
    read_file_result File = PlatformReadEntireFile(FileName);
    unsigned long* Pointer = (unsigned long*)File.Content;

    unsigned long ChunkType = *Pointer++;
    if (ChunkType != 'FFIR') {
        Assert(false);
    }

    unsigned long RIFFChunkSize = *Pointer++;
    unsigned long FileType = *Pointer++;
    if (FileType != 'EVAW') {
        Assert(false);
    }

    ChunkType = *Pointer++;
    if (ChunkType != ' tmf') {
        Assert(false);
    }
    unsigned long ChunkSize = *Pointer++;
    waveformat WaveFMT = *(waveformat*)Pointer;

    Pointer += 4;
    ChunkType = *Pointer++;
    if (ChunkType != 'atad') {
        Assert(false);
    }
    ChunkSize = *Pointer++;

    game_sound Result = { 0 };
    Result.SampleOut = (int16*)Pointer;
    Result.SampleCount = ChunkSize / 2;
    return Result;
}


// Fonts
void LoadFTBMP(FT_Bitmap* SourceBMP, loaded_bmp* DestBMP) {
    DestBMP->Header.Width = SourceBMP->width;
    DestBMP->Header.Height = SourceBMP->rows;
    uint32* DestRow = DestBMP->Content + DestBMP->Header.Width * (DestBMP->Header.Height - 1);
    uint8* Source = SourceBMP->buffer;
    for (int Y = 0; Y < SourceBMP->rows; Y++) {
        uint32* Pixel = DestRow;
        for (int X = 0; X < SourceBMP->width; X++) {
            // FreeType BMPs come with only one byte representing alpha. We load it as a white BMP so changing
            // the color is easier with OpenGL.
            *Pixel++ = (*Source++ << 24) | 0x00ffffff;
        }
        DestRow -= SourceBMP->pitch;
    }
}

character* InitializeFont(memory_arena* Arena, const char* FontPath) {
    FT_Library FTLibrary;
    FT_Face Font;
    FT_Error error = FT_Init_FreeType(&FTLibrary);
    if (error) {
        Assert(false);
    }
    else {
        error = FT_New_Face(FTLibrary, FontPath, 0, &Font);
        if (error == FT_Err_Unknown_File_Format) {
            Assert(false);
        }
        else if (error) {
            Assert(false);
        }
        else {
            // Initializing char bitmaps
            int Points = 20;
            error = FT_Set_Char_Size(Font, 0, Points * 64, 128, 128);
            if (error) {
                Assert(false);
            }

            character* Result = PushArray(Arena, 95, character);
            loaded_bmp* CharacterBMP = PushArray(Arena, 95, loaded_bmp);
            character* pCharacter = Result;
            for (unsigned char c = ' '; c <= '~'; c++) {
                error = FT_Load_Char(Font, c, FT_LOAD_RENDER);
                if (error) {
                    Assert(false);
                }
                else {
                    FT_GlyphSlot Slot = Font->glyph;
                    FT_Bitmap FTBMP = Slot->bitmap;
                    *CharacterBMP = MakeEmptyBitmap(Arena, FTBMP.width, FTBMP.rows, true);
                    LoadFTBMP(&FTBMP, CharacterBMP);

                    character LoadCharacter = { 0 };
                    LoadCharacter.Letter = c;
                    LoadCharacter.Advance = Slot->advance.x >> 6;
                    LoadCharacter.Left = Slot->bitmap_left;
                    LoadCharacter.Top = Slot->bitmap_top;
                    LoadCharacter.Height = Slot->metrics.height;
                    LoadCharacter.Width = Slot->metrics.width;
                    LoadCharacter.Bitmap = CharacterBMP++;
                    LoadCharacter.Bitmap->Handle = 0;

                    *(pCharacter++) = LoadCharacter;
                }
            }
            FT_Done_Face(Font);
            FT_Done_FreeType(FTLibrary);
            return Result;
        }
    }
}


// Video
game_video LoadVideo(memory_arena* Arena, const char* Filename) {
    game_video Result = { 0 };
    Result.VideoContext = PushStruct(Arena, video_context);

    InitializeVideo(Filename, Result.VideoContext);
    int Width = Result.VideoContext->Frame->width;
    int Height = Result.VideoContext->Frame->height;
    Result.VideoContext->VideoOut = PushSize(Arena, Width * Height * 4);

    return Result;
}


// 3D Models
mesh LoadMesh(platform_read_entire_file Read, memory_arena* MeshArena, const char* Path) {
    mesh Result = { 0 };

    read_file_result ReadResult = Read(Path);
    if (ReadResult.ContentSize > 0) {
        char* Pointer = (char*)ReadResult.Content;
        uint32 ReadSize = 0;

        uint32 nVertices = 0;
        uint32 nFaces = 0;
        while (ReadSize < ReadResult.ContentSize) {
            char ReadChar = *Pointer++;
            if (ReadChar == 'v') {
                nVertices++;
            }
            else if (ReadChar == 'f') {
                nFaces++;
            }
            ReadSize++;
        }
        
        Pointer = (char*)ReadResult.Content;
        ReadSize = 0;

        Result.nVertices = nVertices;
        Result.nFaces = nFaces;
        double* VerticesData = PushArray(MeshArena, 8 * nVertices, double);
        uint32* FacesData = PushArray(MeshArena, 3 * nFaces, uint32);

        Result.Vertices = VerticesData;
        Result.Faces = FacesData;

        double* Vertices = VerticesData;
        for (int i = 0; i < nVertices; i++) {
            Pointer += 2;

            char* VXEnd = 0;
            double VX = strtod(Pointer, &VXEnd);
            char* VYEnd = 0;
            double VY = strtod(VXEnd + 1, &VYEnd);
            char* VZEnd = 0;
            double VZ = strtod(VYEnd + 1, &VZEnd);

            char* VNXEnd = 0;
            double VNX = strtod(VZEnd + 1, &VNXEnd);
            char* VNYEnd = 0;
            double VNY = strtod(VNXEnd + 1, &VNYEnd);
            char* VNZEnd = 0;
            double VNZ = strtod(VNYEnd + 1, &VNZEnd);

            char* VTXEnd = 0;
            double VTX = strtod(VNZEnd + 1, &VTXEnd);
            char* VTYEnd = 0;
            double VTY = strtod(VTXEnd + 1, &VTYEnd);

            *Vertices++ = VX;
            *Vertices++ = VY;
            *Vertices++ = VZ;
            *Vertices++ = VNX;
            *Vertices++ = VNY;
            *Vertices++ = VNZ;
            *Vertices++ = VTX;
            *Vertices++ = VTY;

            Pointer = VTYEnd + 1;
        }

        uint32* Faces = FacesData;
        for (int i = 0; i < nFaces; i++) {
            Pointer += 2;

            char* Point1End = 0;
            uint32 Point1 = strtol(Pointer, &Point1End, 10);
            char* Point2End = 0;
            uint32 Point2 = strtol(Point1End + 1, &Point2End, 10);
            char* Point3End = 0;
            uint32 Point3 = strtol(Point2End + 1, &Point3End, 10);

            *Faces++ = Point1;
            *Faces++ = Point2;
            *Faces++ = Point3;

            Pointer = Point3End + 1;
        }

        OutputDebugStringA("Model loaded.\n");
    }
    return Result;
}


/*
mesh LoadMesh(platform_read_entire_file Read, memory_arena* StringsArena, memory_arena* MeshArena, const char* Path) {
    mesh Result = { 0 };
    read_file_result ReadResult = Read(Path);
    if (ReadResult.ContentSize > 50) {
        char* Pointer = (char*)ReadResult.Content;
        uint32 ReadSize = 0;
        char FirstChar;
        char SecondChar;
        char ThirdChar;
        do {
            FirstChar = *Pointer;
            SecondChar = *(Pointer + 1);
            ThirdChar = *(Pointer++ + 2);
            ReadSize++;
        } while ((FirstChar != '\n' || SecondChar != 'o' || ThirdChar != ' ') && ReadSize < ReadResult.ContentSize);
        if (ReadSize + 2 >= ReadResult.ContentSize) {
            OutputDebugStringA("No objects found on .obj file.");
        }
        else {
            Pointer += 2;
            ReadSize += 2;
            for (int i = 0; i < 20; i++) {
                if (*(Pointer + i) == '\n') {
                    Result.Name = PushString(StringsArena, i, Pointer);
                    break;
                }
            }
            do { ReadSize++; } while (*Pointer++ != '\n');
            char Prefix[2] = { *Pointer++, *Pointer++ };
            while (ReadSize < ReadResult.ContentSize) {
                Result.Vertices = (v3*)(MeshArena->Base + MeshArena->Used);
                while (Prefix[0] == 'v' && Prefix[1] == ' ') {
                    char* XEnd = 0;
                    double X = strtod(Pointer, &XEnd);
                    char* YEnd = 0;
                    double Y = strtod(XEnd + 1, &YEnd);
                    char* ZEnd = 0;
                    double Z = strtod(YEnd + 1, &ZEnd);
                    v3* Vertex = PushStruct(MeshArena, v3);
                    *Vertex = V3(X, Y, Z);
                    Result.nVertices++;
                    while (*Pointer++ != '\n') {
                        ReadSize++;
                    }
                    Prefix[0] = *Pointer++;
                    Prefix[1] = *Pointer++;
                    ReadSize += 3;
                }
                Result.TextureVertices = (v2*)(MeshArena->Base + MeshArena->Used);
                while (Prefix[0] == 'v' && Prefix[1] == 't') {
                    Pointer++;
                    ReadSize++;
                    char* XEnd = 0;
                    double X = strtod(Pointer, &XEnd);
                    char* YEnd = 0;
                    double Y = strtod(XEnd + 1, &YEnd);
                    v2* Vertex = PushStruct(MeshArena, v2);
                    *Vertex = V2(X, Y);
                    Result.nTextureVertices++;
                    while (*Pointer++ != '\n') {
                        ReadSize++;
                    }
                    Prefix[0] = *Pointer++;
                    Prefix[1] = *Pointer++;
                    ReadSize += 3;
                }
                Result.Normals = (v3*)(MeshArena->Base + MeshArena->Used);
                while (Prefix[0] == 'v' && Prefix[1] == 'n') {
                    char* XEnd = 0;
                    double X = strtod(Pointer, &XEnd);
                    char* YEnd = 0;
                    double Y = strtod(XEnd + 1, &YEnd);
                    char* ZEnd = 0;
                    double Z = strtod(YEnd + 1, &ZEnd);
                    v3* Vertex = PushStruct(MeshArena, v3);
                    *Vertex = V3(X, Y, Z);
                    Result.nNormals++;
                    while (*Pointer++ != '\n') {
                        ReadSize++;
                    }
                    Prefix[0] = *Pointer++;
                    Prefix[1] = *Pointer++;
                    ReadSize += 3;
                }
                while (Prefix[0] != 'f') {
                    Prefix[0] = *Pointer++;
                    Prefix[1] = *Pointer++;
                    ReadSize += 2;
                }

                char* FaceCountPointer = Pointer;
                uint32 FaceCountReadSize = ReadSize;
                char ReadChar = *FaceCountPointer;
                int FaceCount = 0;
                while (ReadChar == 'f') {
                    FaceCount++;
                    FaceCountPointer += 2;
                    FaceCountReadSize += 2;
                    ReadChar = *FaceCountPointer;
                    int CountSpaces = 0;
                    while (ReadChar != '\n' && FaceCountReadSize < ReadResult.ContentSize) {
                        if (ReadChar == ' ') {
                            CountSpaces++;
                        }
                        ReadChar = *FaceCountPointer++;
                        FaceCountReadSize++;
                    }
                    if (FaceCountReadSize >= ReadResult.ContentSize) {
                        Assert(false);
                    }
                    if (CountSpaces == 3) FaceCount++;
                    FaceCountReadSize++;
                    ReadChar = *FaceCountPointer++;
                }

                Result.nFaces = FaceCount;

                Result.FaceVertices = PushArray(MeshArena, 3 * FaceCount, uint32);
                Result.FaceTextures = PushArray(MeshArena, 3 * FaceCount, uint32);
                Result.FaceNormals = PushArray(MeshArena, FaceCount, uint32);

                uint32* FaceVertices = Result.FaceVertices;
                uint32* FaceTextures = Result.FaceTextures;
                uint32* FaceNormals = Result.FaceNormals;

                Pointer += 2;
                ReadSize += 2;
                while (ReadSize < ReadResult.ContentSize) {
                    uint32 Vertices[4] = { 0 };
                    uint32 Textures[4] = { 0 };
                    uint32 Normal = 0;
                    int Size = 0;
                    for (int i = 0; i < 4; i++) {
                        char* EndV = 0;
                        Vertices[i] = (uint32)strtol(Pointer, &EndV, 10);
                        char* EndVT = 0;
                        Textures[i] = strtol(EndV + 1, &EndVT, 10);
                        char* EndVN = 0;
                        Normal = strtol(EndVT + 1, &EndVN, 10);
                        Size++;
                        bool Break = false;
                        while (*Pointer != ' ') {
                            if (*Pointer == '\n') {
                                Break = true;
                                Pointer += 3;
                                ReadSize += 3;
                                break;
                            }
                            Pointer++;
                            ReadSize++;
                        }
                        if (Break) break;
                        else Pointer++; ReadSize++;
                    }

                    for (int i = 0; i < 3; i++) {
                        *FaceVertices++ = Vertices[i] - 1;
                        *FaceTextures++ = Textures[i] - 1;
                    }
                    *FaceNormals++ = Normal;

                    if (Size == 4) {
                        int Indices[3] = { 2, 0, 3 };
                        for (int i = 0; i < 3; i++) {
                            *FaceVertices++ = Vertices[Indices[i]] - 1;
                            *FaceTextures++ = Textures[Indices[i]] - 1;
                        }
                        *FaceNormals++ = Normal;
                    }
                }
            }
        }
    }
    return Result;
}
*/



void LoadAssets(
    game_assets* Assets,
    platform_api* Platform,
    memory_arena* RenderArena,
    memory_arena* StringsArena,
    memory_arena* FontsArena,
    memory_arena* VideoArena,
    memory_arena* MeshArena
) {
    Assets->Characters = InitializeFont(FontsArena, "C:/Windows/Fonts/CascadiaMono.ttf");

    // Strings
    StringsArena->Name = PushString(StringsArena, 13, "Strings Arena");
    StringsArena->Percentage = PushString(StringsArena, 7, "0.0%");
    FontsArena->Name = PushString(StringsArena, 12, "Fonts Arena");
    FontsArena->Percentage = PushString(StringsArena, 7, "0.0%");
    RenderArena->Name = PushString(StringsArena, 13, "Render Arena");
    RenderArena->Percentage = PushString(StringsArena, 7, "0.0%");
    VideoArena->Name = PushString(StringsArena, 13, "Video Arena");
    VideoArena->Percentage = PushString(StringsArena, 7, "0.0%");
    MeshArena->Name = PushString(StringsArena, 11, "Mesh Arena");
    MeshArena->Percentage = PushString(StringsArena, 7, "0.0%");

    Assets->EmptyTexture = LoadBMP(Platform->ReadEntireFile, "../../GameLibrary/Assets/Bitmaps/Empty.bmp");

    Assets->FaceMesh = LoadMesh(Platform->ReadEntireFile, MeshArena, "../../GameLibrary/Assets/Models/Plane.mdl");

    Assets->FaceMesh.Texture = &Assets->EmptyTexture;

// Shaders
    // Header file
    read_file_result HeaderCode = Platform->ReadEntireFile("../../GameLibrary/Assets/Shaders/HeaderShader.h.glsl");

    // Vertex shaders files
    read_file_result VertexCode = Platform->ReadEntireFile("../../GameLibrary/Assets/Shaders/VertexShader.vert.glsl");
    read_file_result FramebufferVertexCode = Platform->ReadEntireFile("../../GameLibrary/Assets/Shaders/FramebufferVertexShader.vert.glsl");

    // Fragment shaders files
    read_file_result HeightFragmentCode = Platform->ReadEntireFile("../../GameLibrary/Assets/Shaders/HeightFragmentShader.frag.glsl");
    read_file_result SphereFragmentCode = Platform->ReadEntireFile("../../GameLibrary/Assets/Shaders/SphereFragmentShader.frag.glsl");
    read_file_result FramebufferFragmentCode = Platform->ReadEntireFile("../../GameLibrary/Assets/Shaders/FramebufferFragmentShader.frag.glsl");
    read_file_result SingleColorFragmentCode = Platform->ReadEntireFile("../../GameLibrary/Assets/Shaders/SingleColorFragmentShader.frag.glsl");
    read_file_result OutlineInit = Platform->ReadEntireFile("../../GameLibrary/Assets/Shaders/OutlineInitFragmentShader.frag.glsl");
    read_file_result JumpFloodShaderCode = Platform->ReadEntireFile("../../GameLibrary/Assets/Shaders/JumpFloodFragmentShader.frag.glsl");
    read_file_result OutlineShaderCode = Platform->ReadEntireFile("../../GameLibrary/Assets/Shaders/OutlineFragmentShader.frag.glsl");
    read_file_result KernelShaderCode = Platform->ReadEntireFile("../../GameLibrary/Assets/Shaders/KernelFragmentShader.frag.glsl");

    Assets->HeightShader = {
        0,
        HeaderCode,
        VertexCode,
        HeightFragmentCode
    };

    Assets->SphereShader = {
        0,
        HeaderCode,
        VertexCode,
        SphereFragmentCode
    };

    Assets->FramebufferShader = {
        0,
        HeaderCode,
        FramebufferVertexCode,
        FramebufferFragmentCode
    };

    Assets->SingleColorShader = {
        0,
        HeaderCode,
        VertexCode,
        SingleColorFragmentCode
    };

    Assets->OutlineInitShader = {
        0,
        HeaderCode,
        FramebufferVertexCode,
        OutlineInit
    };

    Assets->OutlineShader = {
        0,
        HeaderCode,
        FramebufferVertexCode,
        OutlineShaderCode
    };

    Assets->JumpFloodShader = {
        0,
        HeaderCode,
        FramebufferVertexCode,
        JumpFloodShaderCode
    };

    Assets->KernelShader = {
        0,
        HeaderCode,
        FramebufferVertexCode,
        KernelShaderCode
    };
}
