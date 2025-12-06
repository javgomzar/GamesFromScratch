#include "GamePlatform.h"
#include "GameRender.h"

// void TestFluid(render_group* Group, game_input* Input, bool FirstFrame) {
//     if (FirstFrame || Input->Keyboard.R.JustPressed) {
//         PushShaderPass(Group, Compute_Shader_Fluid_Init_ID, Target_Fluid, Target_Fluid);
//     }
//     else {
//         PushShaderPass(Group, Compute_Shader_Fluid_ID, Target_Fluid, Target_Fluid);
//     }
//     PushRenderTarget(Group, Target_Fluid);
// }

void TestSky(float Time, light* Light) {
    Light->Direction = V3(-cos(0.2f * Time), -sin(0.2f * Time), 0);
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
    // PushText(Group, V2(150, 150 + GetCharMaxHeight(Font, Points)), Font_Menlo_Regular_ID, TestString, White, Points, Options);

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
    transform T = Transform(V3(3, 0, 0), Quaternion(Pi, V3(0,1,0)));
    PushMesh(Group, Mesh_Body_ID, T, Bitmap_Empty_ID, Gray, 0, false);

    T.Translation = V3(-2, 0, 0);
    PushMesh(Group, Mesh_Tetrahedron_ID, T);

    T.Translation = V3(-2, 0, 2);
    PushMesh(Group, Mesh_Cube_ID, T);

    T.Translation = V3(-2, 0, 4);
    PushMesh(Group, Mesh_Octahedron_ID, T);

    T.Translation = V3(-2, 0, 6);
    PushMesh(Group, Mesh_Icosahedron_ID, T);

    T.Translation = V3(-2, 0, 8.5);
    PushMesh(Group, Mesh_Dodecahedron_ID, T);

    PushMesh(Group, Mesh_Sphere_ID, Transform(V3(10, 0, 0)), Bitmap_Empty_ID, Red);

    // Heightmap
    PushHeightmap(Group, Heightmap_Spain_ID);
}
