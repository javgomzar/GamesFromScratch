#include "GamePlatform.h"
#include "GameRender.h"

// void TestTriangleIntersection(render_group* Group, game_input* Input) {
//     triangle2 Test1 = {
//         V2(300, 300),
//         V2(300, 400),
//         V2(400, 200)
//     };

//     triangle2 Test2 = {
//         V2(300, 300),
//         V2(300, 400),
//         Input->Mouse.Cursor
//     };

//     PushTriangle(Group, Test1, Green);
//     PushTriangle(Group, Test2, Intersect(Test1, Test2) ? Magenta : Red);
// }

void TestTriangulations(render_group* Group, game_input* Input) {
    game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    game_font_character* Character = &Font->Characters['c' - '!'];
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Solid_Text_ID);

    render_primitive_options Options = {};
    Options.Font = Font;
    Options.Pen = V2(500, 500);
    Options.TextSize = 0.1;

    color Color = ChangeAlpha(Red, 0.5f);
    uint32 Offset = Character->SolidTrianglesOffset;

    render_primitive_command* Command = PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        Color,
        Shader,
        vertex_layout_vec2_vec2_id,
        0,
        3 * Character->nSolidTriangles,
        SORT_ORDER_DEBUG_OVERLAY,
        Options
    );

    Command->ElementEntry.Offset = Character->SolidTrianglesOffset;

    for (int i = 0; i < Character->nPoints; i++) {
        PushPoint(Group, V2(500, 500) + 0.1 * GetContourPointV2(&Character->Contours[0].Points[i]), HSV2RGB(((float)i)/Character->nPoints, 1.0f, 1.0f));
    }
}

void TestRendering(render_group* Group, game_input* Input) {
// 2D
    // Rects
    rectangle Rect = { 20, 20, 100, 100 };
    PushRect(Group, Rect, Red);
    PushRectOutline(Group, Rect, White, 2.0f);

    // Circles
    PushCircle(Group, V2(200, 70), 50.0f, Red);
    PushCircunference(Group, V2(200, 70), 50.0f, White, 2.0f);

    // Triangles
    triangle2 Triangle2 = { V2(320, 20), V2(270, 120), V2(370, 120) };
    color TriangleColor = Red;
    if (IsInside(Triangle2, Input->Mouse.Cursor)) TriangleColor = Green;
    PushTriangle(Group, Triangle2, TriangleColor);
    PushLine(Group, Triangle2.Points[0], Triangle2.Points[1], White);
    PushLine(Group, Triangle2.Points[1], Triangle2.Points[2], White);
    PushLine(Group, Triangle2.Points[2], Triangle2.Points[0], White);

    // Bitmap
    rectangle BitmapRect = { 20, 140, 100, 200 };
    PushBitmap(Group, Bitmap_Player_ID, BitmapRect);

    // PushText(Group, V2(150, 200), Font_Menlo_Regular_ID, 
    // "ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n?!^/\\(){}[]'\"@#~€$%=+-.,:;*", 
    // White, 50);

// 3D
    // Point
    // PushPoint(Group, V3(0,0,0), Red);

    // Debug grid
    PushDebugGrid(Group, 1.0f);

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
    PushMesh(Group, Mesh_Body_ID, T, Shader_Pipeline_Mesh_ID, Bitmap_Empty_ID, White, 0, true);

    // Heightmap
    PushHeightmap(Group, Heightmap_Spain_ID, Shader_Pipeline_Heightmap_ID);
}
