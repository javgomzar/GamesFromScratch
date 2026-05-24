#include "GameLibrary.h"
#include "GameData.h"
#include "gltf.h"

/*
    Test types. There are three test types:
        - `test_once`: This test type is only run once (when the program loads)
        - `test_reload`: This test type is run every time a new version of the code is hot-reloaded
        - `test_every_frame`: This test type is run every frame
*/
ENUM(test_type,
    test_once,
    test_reload,
    test_every_frame
);

#define TEST(Name, Type, ...) bool Name

TEST(TestFormat, test_once, ACTIVE)() {
    string Test;
    memory_arena Arena = AllocateMemoryArena(1024);

    int32 Int32 = 0;
    Test = Format(&Arena, "Test {i} ...", 1, Int32);
    Assert(Test == string("Test 0 ..."), "Int32 formatting failed.");
    Int32 = -112312492;
    Test = Format(&Arena, "Test {i} ...", 1, Int32);
    Assert(Test == string("Test -112312492 ..."), "Int32 formatting failed.");

    ClearArena(&Arena);

    int64 Int64 = 0;
    Test = Format(&Arena, "Test {I} ...", 1, Int64);
    Assert(Test == string("Test 0 ..."), "Int64 formatting failed.");
    Int64 = -1982938423712312492;
    Test = Format(&Arena, "Test {I} ...", 1, Int64);
    Assert(Test == string("Test -1982938423712312492 ..."), "Int64 formatting failed.");

    ClearArena(&Arena);

    uint32 Uint32 = 0;
    Test = Format(&Arena, "Test {u} ...", 1, Uint32);
    Assert(Test == string("Test 0 ..."), "Uint32 formatting failed.");
    Uint32 = 112312492;
    Test = Format(&Arena, "Test {u} ...", 1, Uint32);
    Assert(Test == string("Test 112312492 ..."), "Uint32 formatting failed.");

    ClearArena(&Arena);

    uint64 Uint64 = 0;
    Test = Format(&Arena, "Test {U} ...", 1, Uint64);
    Assert(Test == string("Test 0 ..."), "Uint64 formatting failed.");
    Uint64 = 1982938423712312492;
    Test = Format(&Arena, "Test {U} ...", 1, Uint64);
    Assert(Test == string("Test 1982938423712312492 ..."), "Uint64 formatting failed.");

    ClearArena(&Arena);

    uint64 Hex = 12379813738877118345ULL;
    Test = Format(&Arena, "Test {x} ...", 1, Hex);
    Assert(Test == "Test 0xabcdef0123456789 ...", "Hex formatting failed.");

    ClearArena(&Arena);

    Test = Format(&Arena, "Hello {s}!", 1, string("world"));
    Assert(Test == string("Hello world!"), "String formatting failed.");

    ClearArena(&Arena);

    double Float = -0.1;
    Test = Format(&Arena, "Test {f3} ...", 1, Float);
    Assert(Test == string("Test -0.100 ..."), "float formatting failed.");

    ClearArena(&Arena);

    Test = Format(&Arena, "Test {i} {I} {u} {U} {s} {f3} ...", 6, Int32, Int64, Uint32, Uint64, string("Hola"), 123.123);
    Assert(Test == string("Test -112312492 -1982938423712312492 112312492 1982938423712312492 Hola 123.123 ..."), "Multiple arguments formatting failed.");
    
    FreeMemoryArena(&Arena);

    return true;
}

TEST(TestFloatingPoint, test_once, ACTIVE)() {
    float TestValues32[] = {
        0.0f,
        -0.0f,
        1.0f,
        69.0f,
        -0.00001f,
        420.0f,
        123456789.9f
    };

    uint32 N = ArrayCount(TestValues32);
    for (int i = 0; i < N; i++) {
        float Value = TestValues32[i];
        int8 Exponent = GetExponent32(Value);
        uint32 Mantissa = GetMantissa32(Value);
        float Result = BuildFloat(Value < 0, Exponent, Mantissa);
        Assert(Value == Result);
    }

    double TestValues64[] = {
        0.0,
        -0.0,
        1.0,
        69.0,
        -0.00001,
        420.0,
        123456789.9
    };

    N = ArrayCount(TestValues64);
    for (int i = 0; i < N; i++) {
        double Value = TestValues64[i];
        int16 Exponent = GetExponent64(Value);
        uint64 Mantissa = GetMantissa64(Value);
        double Result = BuildFloat(Value < 0, Exponent, Mantissa);
        Assert(Value == Result);
    }

    return true;
}

TEST(TestDataFileManager, test_once, ACTIVE)() {
    game_data_file_manager Manager = InitializeDataFileManager("GameData\\Data\\data_file_manager");

    game_data_file* TestFile1 = GetOrCreateDataFile(&Manager, "GameData\\Data\\test_file_1");

    game_data_file* TestFile2 = GetOrCreateDataFile(&Manager, "GameData\\Data\\test_file_1");
    Assert(TestFile2 == TestFile1);

    CloseDataFileManager(Manager);

    return true;
}

TEST(TestData, test_once, ACTIVE)() {
    memory_arena Arena = AllocateMemoryArena(Kilobytes(8));

    game_data_page Page = CreateDataPage(&Arena, row_data_page, 1);

    char TestText[128] = "This is a test text.";
    int TextLength = strlen(TestText);

    game_data_slot* Slot = AddSlot(Page, TextLength);

    uint8* Pointer = (uint8*)Page.Header + Slot->Offset;
    memcpy(Pointer, TestText, TextLength);

    game_data_slot* ReadSlot = GetSlot(Page, Slot->ID);
    char* ReadPointer = (char*)Page.Header + Slot->Offset;

    Log(log_level::Info, ReadPointer);

    Assert(Page.Header->Size == TextLength + sizeof(game_data_page_header) + sizeof(game_data_slot));

    FreeMemoryArena(&Arena);
    Log(log_level::Info, "Data test ended.");

    return true;
}

TEST(TestInstancedRendering, test_every_frame, INACTIVE)(render_group* Group) {
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

    return true;
}

TEST(TestTextRendering, test_every_frame, INACTIVE)(render_group* Group, game_input* Input) {
    render_text_options Options = {};
    Options.Outline = false;
    Options.OutlineWidth = 1.5f;

    static float Points = 42;
    if (Input->Mouse.Wheel > 0) {
        Points *= 1.25f;
    }
    else if (Input->Mouse.Wheel < 0) {
        Points *= 0.8f;
    }

    const char* TestString = "!\"#$%&'()*+,-./0123456789:;<=>?@\nABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`\nabcdefghijklmnopqrstuvwxyz{|}~";
    game_font* Font = GetAsset(Group->Assets, Font_DejaVu_Sans_ID);

    PushText(Group, V2(150, 150 + GetCharMaxHeight(Font, Points)), TestString, .Color = White, .Font = Font->ID, .Outline = false, .Points = Points);

    return true;
}

TEST(TestRendering, test_every_frame, INACTIVE)(render_group* Group, game_input* Input, float Time) {
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
    PushBitmap(Group, Texture_Player_ID, BitmapRect);

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

    return true;
}

TEST(TestSky, test_every_frame, INACTIVE)(float Time, light* Light) {
    Light->Direction = V3(-cos(0.2f * Time), -sin(0.2f * Time), 0);
    return true;
}

// TEST(TestFluid, test_every_frame, INACTIVE)(render_group* Group, game_input* Input, bool FirstFrame) {
//     if (FirstFrame || Input->Keyboard.R.JustPressed) {
//         PushShaderPass(Group, Compute_Shader_Fluid_Init_ID, Target_Fluid, Target_Fluid);
//     }
//     else {
//         PushShaderPass(Group, Compute_Shader_Fluid_ID, Target_Fluid, Target_Fluid);
//     }
//     PushRenderTarget(Group, Target_Fluid);
// }

TEST(TestFFT, test_every_frame, INACTIVE)(render_group* Group, memory_arena* Permanent, float Time) {
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
        Signal = PushArray(Permanent, N, float);
        InputData = PushArray(Permanent, N, complex);
        OutputData = PushArray(Permanent, N, complex);
        Twiddle = PushArray(Permanent, N, complex);
        ModulusDFT = PushArray(Permanent, N, float);
        PhaseDFT = PushArray(Permanent, N, float);
        ModulusFFT = PushArray(Permanent, N, float);
        PhaseFFT = PushArray(Permanent, N, float);

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
    sprintf(TextBuffer, "DFT: %.2f ms", DFTms);
    PushText(Group, V2(600, 320), TextBuffer);
    PushDebugPlot(Group, N, ModulusDFT, V2(600, 200), 1);
    PushDebugPlot(Group, N, PhaseDFT, V2(600, 250), 1);

    sprintf(TextBuffer, "FFT: %.2f ms", FFTms);
    PushText(Group, V2(1000, 320), TextBuffer);
    PushDebugPlot(Group, N, ModulusFFT, V2(1000, 200), 1);
    PushDebugPlot(Group, N, PhaseFFT, V2(1000, 250), 1);

    return true;
}

TEST(TestEntities, test_once, ACTIVE)(game_state* State) {
    game_entity* Character = CreateEntity(State->Entities, "Character", Character_Entity_Type);
    Character->Collider = CapsuleCollider(V3(0,1.0f,0), V3(0,3.6f,0), 1.0f);
    State->ActiveCamera->Follow = Character;

    game_entity* Prop = CreateEntity(State->Entities, "Prop", Prop_Entity_Type);
    Prop->MeshID = Mesh_Sphere_ID;
    Prop->Color = Red;
    Prop->Transform = GetTransform(V3(0,0,5), GetScale(10,1,1));
    Prop->Collider = CapsuleCollider(V3(-0.9f,0,0), V3(0.9f,0,0), 1.0f);

    // enemy* Enemy = AddEnemy(EntityState, V3(10,0,5));

    game_entity* Sword = CreateEntity(State->Entities, "Sword", Weapon_Entity_Type);
    Sword->MeshID = Mesh_Sword_ID;
    Sword->Color = Gray;
    Sword->Transform.Translation = V3(5,0,0);
    Sword->Collider = CapsuleCollider(V3(0,0,0), V3(0,3,0), 0.6f);

    game_entity* Shield = CreateEntity(State->Entities, "Shield", Weapon_Entity_Type);
    Shield->MeshID = Mesh_Shield_ID;
    Shield->Color = Gray;
    Shield->Transform.Translation = V3(10,1.6f,0);
    Shield->Collider = CapsuleCollider(V3(0,-0.5f,0), V3(0,0.5f,0), 1.0f);

    return true;
}

TEST(TestglTF, test_once, ACTIVE, MUST_PASS)() {
    file_info FileInfo;
    char* Content = (char*)Platform.ReadEntireFile("GameAsset/TestFiles/glTF-Sample-Assets-main/Models/Box/glTF/Box.gltf", &FileInfo);

    memory_arena Arena = AllocateMemoryArena(Kilobytes(8));
    ParseGLTF(&Arena, Content);

    return true;
}