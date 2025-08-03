#include "GameRender.h"

void TestRendering(render_group* Group, game_input* Input) {
// 2D
    // Rects
    rectangle Rect = { 20, 20, 100, 100 };
    PushRectOutline(Group, Rect, White, 2.0f);
    PushRect(Group, Rect, Red);

    // Circles
    PushCircunference(Group, V2(200, 70), 50.0f, White);
    PushCircle(Group, V2(200, 70), 50.0f, Red);

    // Triangles
    triangle2 Triangle2 = { V2(320, 20), V2(270, 120), V2(370, 120) };
    PushLine(Group, Triangle2.Points[0], Triangle2.Points[1], White);
    PushLine(Group, Triangle2.Points[1], Triangle2.Points[2], White);
    PushLine(Group, Triangle2.Points[2], Triangle2.Points[0], White);
    color TriangleColor = Red;
    if (IsInside(Triangle2, Input->Mouse.Cursor)) TriangleColor = Green;
    PushTriangle(Group, Triangle2, TriangleColor);

    // Bitmap
    rectangle BitmapRect = { 20, 140, 100, 200 };
    PushBitmap(Group, Bitmap_Player_ID, BitmapRect);

    // Text
    static float Points = 36.0f;

    if (Input->Mouse.Wheel > 0)      Points *= 1.5f;
    else if (Input->Mouse.Wheel < 0) Points /= 1.5f;

    PushText(Group, V2(100, 100), Font_Menlo_Regular_ID, 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n?!^/\\(){}[]'\"@#~€$%=+-.,:;*", 
    White, Points);

    triangle2 TestTriangle1 = {
        V2(300, 300),
        V2(300, 400),
        V2(400, 400)
    };

    static triangle2 TestTriangle2 = { V2(300, 300), V2(300, 400) };
    static bool FollowingMouse = true;
    static int ChangingVertex = 2;

    if (Input->Mouse.RightClick.JustPressed) {
        FollowingMouse = !FollowingMouse;
    }

    if (FollowingMouse) {
        if (Input->Mouse.LeftClick.JustPressed) {
            ChangingVertex = ChangingVertex == 1 ? 2 : 1;
        }
        TestTriangle2[ChangingVertex] = Input->Mouse.Cursor;
    }

    triangle2 Intersections[4] = {};

    int nIntersections = SymmetricDifference(TestTriangle1, TestTriangle2, Intersections);

    color Colors[4] = { Cyan, Magenta, Yellow, Orange };
    for (int i = 0; i < nIntersections; i++) {
        PushTriangle(Group, Intersections[i], Colors[i], SORT_ORDER_DEBUG_OVERLAY - 1);
    }

    char Buffer[16];
    sprintf_s(Buffer, "%d", nIntersections);

    PushText(Group, V2(600, 300), Font_Menlo_Regular_ID, Buffer, White);

    PushTriangle(Group, TestTriangle2, Green);
    PushTriangle(Group, TestTriangle1, Red);

    // Bezier
    // float BezierVertices[] = {
    //     300, 300,
    //     Input->Mouse.Cursor.X, Input->Mouse.Cursor.Y,
    //     400, 400
    // };
    // render_primitive_options Options = {};
    // Options.Color = White;
    // Options.PatchParameter = 3;
    // game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipe);
    // PushPrimitiveCommand(
    //     Group,
    //     Options,
    //     render_primitive_patches,
    //     Shader,
    //     vertex_layout_vec2_id,
    //     3,
    //     BezierVertices,
    //     SORT_ORDER_DEBUG_OVERLAY
    // );

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
