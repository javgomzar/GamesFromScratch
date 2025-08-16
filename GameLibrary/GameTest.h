#include "GamePlatform.h"
#include "GameRender.h"

void TestXArray() {
    xarray<float> TestArray;
    
    for (int i = 0; i < 10000; i++) {
        float TestElement = i;
        TestArray.Insert(TestElement);
        float TestRead = TestArray[i];
        Assert(TestRead == (float)i);
    }

    float FinalXarTest = TestArray[426];
    Assert(FinalXarTest == 426.0f);
    Log(Info, "Test");
}

void TestStack() {
    float TestStack[64] = {};
    stack<float> Stack(64, TestStack);
    Stack.Push(1.0f);
    Stack.Push(-1.0f);
    Stack.Push(3.1415f);
    Stack.Push(69.0f);
    Stack.Push(420.0f);
    Stack.Push(42.0f);

    float F1 = Stack.Pop();
    float F2 = Stack.Pop();
    Stack.Push(-5);
    Stack.Push(-6);
    float F3 = Stack.Pop();
}

void TestTriangleIntersection(render_group* Group, game_input* Input) {
    triangle2 T1 = { V2(300, 300), V2(300, 400), V2(400, 400) };
    static triangle2 T2 = { V2(200, 200), V2(200, 400), V2(400, 400) };

    static bool MoveVertex = false;

    if (Input->Mouse.RightClick.JustPressed) {
        MoveVertex = !MoveVertex;
    }

    if (MoveVertex) {
        T2[2] = Input->Mouse.Cursor;
    }

    PushTriangle(Group, T1, Red);
    PushTriangle(Group, T2, Intersect(T1, T2) ? Green : Magenta);
}

// void TestPointInPolygon(render_group* Group, game_input* Input) {
//     game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
//     game_font_character* Character = &Font->Characters['A' - '!'];

//     memory_arena Arena = AllocateMemoryArena(Kilobytes(32));
//     glyph_contour Contour = Character->Contours[1];

//     game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
//     v2* Vertices = (v2*)PushPrimitiveCommand(
//         Group, { White }, render_primitive_line_loop, Shader, vertex_layout_vec2_id, Contour.nPoints, SORT_ORDER_DEBUG_OVERLAY
//     ).Vertices;
    
//     float Winding = 0;
//     linked_list List = {};
//     for (int i = 0; i < Contour.nPoints; i++) {
//         glyph_contour_point First = Contour.Points[i];
//         v2 Point = 0.3f * V2(First.X, -First.Y) + V2(100, 500);
//         Vertices[i] = Point;

//         link* Link = PushStruct(&Arena, link);
//         Link->Data = &Vertices[i];
//         List.PushBack(Link);
//     }
//     List.CloseCircle();
//     polygon P = {List};

//     char Buffer[128] = {};
//     sprintf_s(Buffer, "%.2f", GetWindingNumber(P, Input->Mouse.Cursor));
//     PushText(Group, V2(100, 500), Font_Menlo_Regular_ID, Buffer);
// }

void TestTriangulations(render_group* Group, game_input* Input) {
    memory_arena TempArena = AllocateMemoryArena(Kilobytes(32));

    game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    ComputeTriangulation(&TempArena, Font);

    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    v2* Vertices = (v2*)PushPrimitiveCommand(
        Group,
        { White },
        render_primitive_triangle,
        Shader,
        vertex_layout_vec2_id,
        TempArena.Used / sizeof(v2),
        SORT_ORDER_DEBUG_OVERLAY
    ).Vertices;

    for (int i = 0; i < 18; i++) {
        Vertices[i] = ((v2*)TempArena.Base)[i];
    }

    FreeMemoryArena(&TempArena);
    /*
    game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    char TestChar = 'e';
    game_font_character* Character = &Font->Characters[TestChar - '!'];

    uint64 Size = Kilobytes(32);
    uint8* Base = (uint8*)VirtualAlloc(0, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    memory_arena Arena = MemoryArena(Size, Base);

    // Text
    static float Points = 615.093750;

    const int nColors = 9;
    color Colors[nColors] = { Red, Orange, Yellow, Green, Cyan, Blue, Purple, Magenta, White };

    if (Input->Mouse.Wheel > 0)      Points *= 1.5f;
    else if (Input->Mouse.Wheel < 0) Points /= 1.5f;

    polygon Polygon = {};
    xarray<triangle2> VoidTriangles;
    bool* Convex = new bool[Character->nContours];
    bool* IsExterior = new bool[Character->nContours];
    // uint64 ContourHeadersSize = sizeof(glyph_contour) + Character->nPoints * sizeof(bool);
    for (int i = 0; i < Character->nContours; i++) {
        glyph_contour* Contour = &;
        Contours += sizeof(glyph_contour) + Contour->nPoints * sizeof(bool);
        
        for (int j = 0; j < Contour->nPoints; j++) {
            bool OnCurve = Contour->IsOnCurve[j];
            if (!OnCurve) {
                v2 A = ContourPoints[j-1];
                v2 B = ContourPoints[j];
                v2 C = {};
                if (i == Character->nContours - 1 && j == Contour->nPoints - 1) {
                    C = *(v2*)Character->Data;
                }
                else {
                    C = ContourPoints[j+1];
                }

                triangle2 T = { A, B, C };
                float Area = GetArea(T);
                if (Area < 0) {
                    VoidTriangles.Insert(T);
                    T[0].X =  0.4f * T[0].X + 501.0f;
                    T[0].Y = -0.4f * T[0].Y + 600.0f;
                    T[1].X =  0.4f * T[1].X + 501.0f;
                    T[1].Y = -0.4f * T[1].Y + 600.0f;
                    T[2].X =  0.4f * T[2].X + 501.0f;
                    T[2].Y = -0.4f * T[2].Y + 600.0f;   
                }
                else {
                    if (j == Contour->nPoints-1) {
                        T[2] = ContourPoints[0];
                    }
                    T[0].X =  0.4f * T[0].X + 501.0f;
                    T[0].Y = -0.4f * T[0].Y + 600.0f;
                    T[1].X =  0.4f * T[1].X + 501.0f;
                    T[1].Y = -0.4f * T[1].Y + 600.0f;
                    T[2].X =  0.4f * T[2].X + 501.0f;
                    T[2].Y = -0.4f * T[2].Y + 600.0f;
                    float Vertices[15] = {
                        T[0].X, T[0].Y, 0, 0,
                        T[1].X, T[1].Y, 1, 0,
                        T[2].X, T[2].Y, 0, 1
                    };

                    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Bezier_Exterior_ID);
                    PushPrimitiveCommand(
                        Group,
                        { ChangeAlpha(Cyan, 0.8f) },
                        render_primitive_triangle,
                        Shader,
                        vertex_layout_vec2_vec2_id,
                        3,
                        Vertices,
                        SORT_ORDER_DEBUG_OVERLAY
                    );

                    PointData++;
                    continue;
                }
            }

            link* Link = PushStruct(&Arena, link);
            Link->Data = PointData;
            Polygon.Vertices.PushBack(Link);
            
            PointData++;
        }

        link* Link = PushStruct(&Arena, link);
        Link->Data = ContourPoints;
        Polygon.Vertices.PushBack(Link);

        Convex[i] = Contour->IsConvex;
        IsExterior[i] = Contour->IsExterior;        
    }

    for (int i = 0; i < VoidTriangles.Size(); i++) {
        triangle2 T = VoidTriangles[i];
        T[0].X =  0.4f * T[0].X + 501.0f;
        T[0].Y = -0.4f * T[0].Y + 600.0f;
        T[1].X =  0.4f * T[1].X + 501.0f;
        T[1].Y = -0.4f * T[1].Y + 600.0f;
        T[2].X =  0.4f * T[2].X + 501.0f;
        T[2].Y = -0.4f * T[2].Y + 600.0f;
        float Vertices[15] = {
            T[0].X, T[0].Y, 0, 0,
            T[1].X, T[1].Y, 1, 0,
            T[2].X, T[2].Y, 0, 1
        };

        game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Bezier_Interior_ID);
        PushPrimitiveCommand(
            Group,
            { ChangeAlpha(Cyan, 0.8f) },
            render_primitive_triangle,
            Shader,
            vertex_layout_vec2_vec2_id,
            3,
            Vertices,
            SORT_ORDER_DEBUG_OVERLAY
        );
    }

    Polygon.Vertices.CloseCircle();
    link* Vertex = Polygon.Vertices.First;
    uint8 RenderedTriangles = 0;
    uint64 VertexCount = CountVertices(Polygon);
    for (int i = 0; i < VertexCount - 2; i++) {
        v2 A = {}, B = {}, C = {}; 
        triangle2 T = {};
        while (true) {
            A = *(v2*)Vertex->Previous->Data;
            B = *(v2*)Vertex->Data;
            C = *(v2*)Vertex->Next->Data;

            T = { A, B, C };
            float Area = GetArea(T);
            
            if (Area > 0) {
                bool Valid = true;
                for (int j = 0; j < VoidTriangles.Size(); j++) {
                    triangle2 Void = VoidTriangles[j];

                    if (Intersect(Void, T)) {
                        Valid = false;
                        break;
                    }
                }

                if (Valid) break;
            }

            Vertex = Vertex->Next;
        }

        A.X =  0.4f * A.X + 501.0f;
        A.Y = -0.4f * A.Y + 600.0f;
        B.X =  0.4f * B.X + 501.0f;
        B.Y = -0.4f * B.Y + 600.0f;
        C.X =  0.4f * C.X + 501.0f;
        C.Y = -0.4f * C.Y + 600.0f;

        T = { A, B, C };
        color Color = Colors[i % nColors];
        PushTriangle(Group, T, ChangeAlpha(Cyan, 0.8f));
        //PushPoint(Group, B, Color);
        
        link* Next = Vertex->Next;
        Polygon.Vertices.Break(Vertex);
        Vertex = Next;
    }

    delete [] Convex;
    delete [] IsExterior;

    char Buffer[2] = { TestChar };
    PushText(Group, V2(550,600), Font_Menlo_Regular_ID, Buffer, 
    Cyan, Points);

    */
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
