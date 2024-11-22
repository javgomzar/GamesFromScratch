#include "GamePlatform.h"
#include "GameMath.h"

// Freetype
#include "ft2build.h"
#include FT_FREETYPE_H

// FFMPEG
#include "FFMpeg.h"

#ifndef GAME_ASSETS
#define GAME_ASSETS


// Sound
struct game_sound {
    uint32 SampleCount;
    uint32 Played;
    int16* SampleOut;
};

struct waveformat {
    unsigned short    wFormatTag;        /* format type */
    unsigned short    nChannels;         /* number of channels (i.e. mono, stereo...) */
    unsigned long     nSamplesPerSec;    /* sample rate */
    unsigned long     nAvgBytesPerSec;   /* for buffer estimation */
    unsigned short    nBlockAlign;       /* block size of data */
    unsigned short    wBitsPerSample;    /* Number of bits per sample of mono data */
    unsigned short    cbSize;            /* The count in bytes of the size of extra information (after cbSize) */
};

game_sound LoadWAV(platform_read_entire_file* PlatformReadEntireFile, const char* FileName);


// Bitmaps
struct loaded_bmp {
    bitmap_header Header;
    uint32 Handle;
    uint32 BytesPerPixel;
    uint32 Pitch;
    uint32 AlphaMask;
    uint32* Content;
};

loaded_bmp LoadBMP(platform_read_entire_file* PlatformReadEntireFile, const char* Path);


// Fonts
struct character {
    unsigned char Letter;
    signed long Advance;
    signed long Width;
    signed long Height;
    int Left;
    int Top;
    loaded_bmp* Bitmap;
};

// Buttons
struct button {
    bool Clicked;
    bool Active;
    game_rect Collider;
    loaded_bmp Image;
    loaded_bmp ClickedImage;
    string Text;
};

// Video
struct game_video {
    video_context* VideoContext;
    int Handle;
    bool Loop;
    double TimeElapsed;
};

// 3D Models
struct vertex {
    v3 Vertex;
    v3 Normal;
    v2 Texture;
};

struct mesh {
    int nVertices;
    double* Vertices;
    int nFaces;
    uint32* Faces;
    uint32 VBO;
    uint32 VAO;
    uint32 EBO;
    loaded_bmp* Texture;
};

mesh LoadMesh(platform_read_entire_file Read, memory_arena* MeshArena, const char* Path);


// Shaders
struct shader {
    uint32 ProgramID;
    read_file_result HeaderShaderCode;
    read_file_result VertexShaderCode;
    read_file_result FragmentShaderCode;
};

// Game Assets
struct game_assets {
    character* Characters;
    loaded_bmp TestImage;
    loaded_bmp TestImage2;
    loaded_bmp EmptyTexture;
    game_sound TestSound;
    mesh TestMesh;
    mesh TestMesh2;
    shader TextureShader;
    shader SphereShader;
    shader FramebufferShader;
    shader SingleColorShader;
    shader OutlineInitShader;
    shader JumpFloodShader;
    shader OutlineShader;
    shader KernelShader;
    shader AntialiasingShader;
    game_video TestVideo;
};

void LoadAssets(
    game_assets* Assets,
    platform_api* Platform,
    memory_arena* RenderArena,
    memory_arena* StringsArena,
    memory_arena* FontsArena,
    memory_arena* VideoArena,
    memory_arena* MeshArena
);

#endif