#include "GamePlatform.h"
#include "GameRender.h"

/*
void TestTriangulations(render_group* Group, game_input* Input) {
    memory_arena SolidArena = AllocateMemoryArena(Kilobytes(32));
    memory_arena InteriorArena = AllocateMemoryArena(Kilobytes(32));
    memory_arena ExteriorArena = AllocateMemoryArena(Kilobytes(32));
    memory_arena VoidArena = AllocateMemoryArena(Kilobytes(32));
    memory_arena TrialArena = AllocateMemoryArena(Kilobytes(32));

    game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    // glyph_triangulation Triangulation = ComputeTriangulation(&SolidArena, &InteriorArena, &ExteriorArena, &VoidArena, &TrialArena, Font);

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    v2* Vertices = (v2*)PushPrimitiveCommand(
        Group,
        render_primitive_triangle,
        White,
        Shader,
        vertex_layout_vec2_id,
        3 * Triangulation.nSolid,
        0,
        SORT_ORDER_DEBUG_OVERLAY
    )->Vertices;

    memcpy(Vertices, SolidArena.Base, SolidArena.Used);

    // float* VoidTriangles = PushPrimitiveCommand(
    //     Group, { ChangeAlpha(Black, 0.5f) },
    //     render_primitive_triangle,
    //     Shader,
    //     vertex_layout_vec2_id,
    //     3 * Triangulation.nVoid,
    //     SORT_ORDER_DEBUG_OVERLAY
    // ).Vertices;

    // memcpy(VoidTriangles, VoidArena.Base, VoidArena.Used);

    game_shader_pipeline* InteriorShader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Bezier_Interior_ID);
    float* InteriorTriangles = PushPrimitiveCommand(
        Group, 
        render_primitive_triangle,
        White,
        InteriorShader,
        vertex_layout_vec2_vec2_id,
        3 * Triangulation.nInterior,
        0,
        SORT_ORDER_DEBUG_OVERLAY
    )->Vertices;

    memcpy(InteriorTriangles, InteriorArena.Base, InteriorArena.Used);

    game_shader_pipeline* ExteriorShader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Bezier_Exterior_ID);
    float* ExteriorTriangles = PushPrimitiveCommand(
        Group, 
        render_primitive_triangle,
        White,
        ExteriorShader,
        vertex_layout_vec2_vec2_id,
        3 * Triangulation.nExterior,
        0,
        SORT_ORDER_DEBUG_OVERLAY
    )->Vertices;

    memcpy(ExteriorTriangles, ExteriorArena.Base, ExteriorArena.Used);

    // uint32 nTrials = TrialArena.Used / sizeof(v2);
    // for (int i = 0; i < nTrials; i++) {
    //     v2* Trial = (v2*)PushPrimitiveCommand(
    //         Group, { HSV2RGB((float)i/(float)nTrials, 1.0f, 1.0f) },
    //         render_primitive_point,
    //         Shader,
    //         vertex_layout_vec2_id,
    //         1,
    //         SORT_ORDER_DEBUG_OVERLAY
    //     ).Vertices;
    //     *Trial = ((v2*)TrialArena.Base)[i];
    // }

    FreeMemoryArena(&SolidArena);
    FreeMemoryArena(&InteriorArena);
    FreeMemoryArena(&ExteriorArena);
    FreeMemoryArena(&VoidArena);
    FreeMemoryArena(&TrialArena);
}
*/

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

    PushText(Group, V2(150, 200), Font_Menlo_Regular_ID, 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n?!^/\\(){}[]'\"@#~€$%=+-.,:;*", 
    White, 50);

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
