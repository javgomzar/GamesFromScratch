#include "GamePlatform.h"
#include "GameRender.h"

void TestInstancedRendering(render_group* Group) {
    int nInstances = 100;
    render_primitive_command* Command = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        vertex_layout_v2_id,
        6,
        .InstanceLayoutID = vertex_layout_v2_id,
        .nInstances = nInstances,
        .Order = SORT_ORDER_DEBUG_OVERLAY
    );

    float* Vertices = Command->Vertices;
    *Vertices++ = 0.0; *Vertices++ = 0.0;
    *Vertices++ = 0.0; *Vertices++ = 1.0;
    *Vertices++ = 1.0; *Vertices++ = 0.0;
    *Vertices++ = 1.0; *Vertices++ = 1.0;
    *Vertices++ = 1.0; *Vertices++ = 0.0;
    *Vertices++ = 0.0; *Vertices++ = 1.0;

    float* Instances = (float*)Command->InstanceEntry.Pointer;
    for (int i = 0; i < 10; i++)
    for (int j = 0; j < 10; j++) {
        *Instances++ = i * Group->Width / 10;
        *Instances++ = j * Group->Height / 10;
    }
}

void TestRendering(render_group* Group, game_input* Input, float Time) {
// 2D
    // Rects
    rectangle Rect = { 20, 20, 100, 100 };
    PushRect(Group, Rect, Red);
    PushRectOutline(Group, Rect, White, 2.0f);

    // Circles
    PushCircle(Group, V2(200, 70), 50.0f, Red);
    PushCircunference(Group, V2(200, 70), 50.0f, White, 2.0f);

    // Triangles
    triangle2 Triangle2 = { V2(320, 20), V2(370, 120), V2(270, 120) };
    color TriangleColor = Red;
    if (IsInside(Triangle2, Input->Mouse.Cursor)) TriangleColor = Green;
    PushTriangle(Group, Triangle2, TriangleColor);
    PushLine(Group, Triangle2.Points[0], Triangle2.Points[1], White);
    PushLine(Group, Triangle2.Points[1], Triangle2.Points[2], White);
    PushLine(Group, Triangle2.Points[2], Triangle2.Points[0], White);

    // Bitmap
    rectangle BitmapRect = { 20, 140, 100, 200 };
    PushBitmap(Group, Bitmap_Player_ID, BitmapRect);

    static float Points = 42;
    if (Input->Mouse.Wheel > 0) {
        Points *= 1.25f;
    }
    else if (Input->Mouse.Wheel < 0) {
        Points *= 0.8f;
    }

    // Text
    // render_text_options Options = {};
    // Options.Outline = false;
    // Options.OutlineWidth = 1.5f;

    // const char* TestString = "!\"#$%&'()*+,-./0123456789:;<=>?@\nABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`\nabcdefghijklmnopqrstuvwxyz{|}~";
    // game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    // PushText(Group, V2(150, 150 + GetCharMaxHeight(Font, Points)), TestString, .Outline = false, .Points = Points);

// 3D
    // Point
    // PushPoint(Group, V3(0,0,0), Red);

    // Rect
    PushRect(Group, V3(0,1,0), V3(0,0,1), V3(0,1,0), 1.0f, 1.0f, Red);

    // Circle
    PushCircle(Group, V3(0,1.5f,2), 0.5f, V3(0,0,1), Red);
    
    // Triangle
    triangle3 Triangle3 = {};
    Triangle3.Points[0] = V3(0, 2, 3.25);
    Triangle3.Points[1] = V3(0, 1, 2.75);
    Triangle3.Points[2] = V3(0, 1, 3.75);
    PushTriangle(Group, Triangle3, Red);

    // Mesh
    transform Transform = GetTransform(V3(3, 0, 0), Quaternion(Pi, V3(0,1,0)));
    PushMesh(Group, Mesh_Body_ID, .Color = Gray, .Transform = Transform, .Outline = true);

    Transform.Translation = V3(-2, 0, 0);
    PushMesh(Group, Mesh_Tetrahedron_ID, .Transform = Transform);

    Transform.Translation = V3(-2, 0, 2);
    PushMesh(Group, Mesh_Cube_ID, .Transform = Transform);

    Transform.Translation = V3(-2, 0, 4);
    PushMesh(Group, Mesh_Octahedron_ID, .Transform = Transform);

    Transform.Translation = V3(-2, 0, 6);
    PushMesh(Group, Mesh_Icosahedron_ID, .Transform = Transform);

    Transform.Translation = V3(-2, 0, 8.5);
    PushMesh(Group, Mesh_Dodecahedron_ID, .Transform = Transform);

    PushMesh(Group, Mesh_Sphere_ID, .Color = Red, .Transform = GetTransform(V3(10, 0, 0)));

    // Heightmap
    PushHeightmap(Group, Heightmap_Spain_ID, V3(0,0,0), GetScale(10, 1, 10));
}

void TestSky(float Time, light* Light) {
    Light->Direction = V3(-cos(0.2f * Time), -sin(0.2f * Time), 0);
}

// void TestFluid(render_group* Group, game_input* Input, bool FirstFrame) {
//     if (FirstFrame || Input->Keyboard.R.JustPressed) {
//         PushShaderPass(Group, Compute_Shader_Fluid_Init_ID, Target_Fluid, Target_Fluid);
//     }
//     else {
//         PushShaderPass(Group, Compute_Shader_Fluid_ID, Target_Fluid, Target_Fluid);
//     }
//     PushRenderTarget(Group, Target_Fluid);
// }

void TestFFT(render_group* Group, memory_arena* Arena, float Time) {
    uint32 N = 256;
    
    static bool Initialized = false;
    static float* Signal = nullptr;
    static complex* InputData = nullptr;
    static complex* OutputData = nullptr;
    static complex* Twiddle = nullptr;
    static float* ModulusDFT = nullptr;
    static float* PhaseDFT = nullptr;
    static float* ModulusFFT = nullptr;
    static float* PhaseFFT = nullptr;

    if (!Initialized) {
        Signal = PushArray(Arena, N, float);
        InputData = PushArray(Arena, N, complex);
        OutputData = PushArray(Arena, N, complex);
        Twiddle = PushArray(Arena, N, complex);
        ModulusDFT = PushArray(Arena, N, float);
        PhaseDFT = PushArray(Arena, N, float);
        ModulusFFT = PushArray(Arena, N, float);
        PhaseFFT = PushArray(Arena, N, float);

        for (int i = 0; i < N; i++) {
            Signal[i] = sin(0.333333f*i);
            InputData[i].r = Signal[i];
            InputData[i].i = 0.0f;
        }

        PrepareTwiddleFactors(N, Twiddle);

        Initialized = true;
    }

    uint64 Start = Platform.GetWallClock();
    DFT(N, InputData, OutputData);
    uint64 End = Platform.GetWallClock();

    float DFTms = 1000.0f * GetSecondsElapsed(Start, End);

    for (int i = 0; i < N; i++) {
        ModulusDFT[i] = modulus(OutputData[i]);
        PhaseDFT[i] = 5.0f * phase(OutputData[i]);
    }

    Start = Platform.GetWallClock();
    FFT(N, InputData, OutputData, Twiddle);
    End = Platform.GetWallClock();

    for (int i = 0; i < N; i++) {
        ModulusFFT[i] = modulus(OutputData[i]);
        PhaseFFT[i] = 5.0f * phase(OutputData[i]);
    }

    float FFTms = 1000.0f * GetSecondsElapsed(Start, End);
    
    PushText(Group, V2(200, 320), "Signal");
    PushDebugPlot(Group, N, Signal, V2(200, 200), 1);
    
    char TextBuffer[128];
    sprintf_s(TextBuffer, "DFT: %.2f ms", DFTms);
    PushText(Group, V2(600, 320), TextBuffer);
    PushDebugPlot(Group, N, ModulusDFT, V2(600, 200), 1);
    PushDebugPlot(Group, N, PhaseDFT, V2(600, 250), 1);

    sprintf_s(TextBuffer, "FFT: %.2f ms", FFTms);
    PushText(Group, V2(1000, 320), TextBuffer);
    PushDebugPlot(Group, N, ModulusFFT, V2(1000, 200), 1);
    PushDebugPlot(Group, N, PhaseFFT, V2(1000, 250), 1);
}