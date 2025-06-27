#ifndef GAME_UI
#define GAME_UI

#include "GameLibrary.h"
#include <vector>

/*
    TODO:
        - Save game state in files.
        - Load game state from files.
        - Debug fps graphs.
        - Log debug records as a table.
        - Debug entities.
*/

enum ui_axis {
    axis_x,
    axis_y
};

enum ui_alignment {
    ui_alignment_center,
    ui_alignment_min,
    ui_alignment_max
};

/*
    Returns the relative offset of a `Contained` length when fitting it inside a `Container` following `Alignment`.
        - Example: `Align(ui_alignment_center, 100.0f, 50.0f) == 25.0f`
*/
float Align(ui_alignment Alignment, float Container, float Contained, float Margin) {
    Assert(Container >= Contained);
    switch(Alignment) {
        case ui_alignment_center: { return 0.5f * (Container - Contained); } break;
        case ui_alignment_min:    { return Margin; } break;
        case ui_alignment_max:    { return Container - Contained; } break;
        default: Raise("Invalid alignment.");
    }
    return 0;
}

enum ui_size_type {
    ui_size_null,
    ui_size_pixels,
    ui_size_text,
    ui_size_percent_of_parent,
    ui_size_sum_of_children,
    ui_size_max_of_children
};

struct ui_size {
    ui_size_type Type;
    float Value;
};

ui_size UISizeNull() {
    return { ui_size_null, 0.0f };
}

ui_size UISizePixels(float Value) {
    return { ui_size_pixels, Value };
}

ui_size UISizePercentParent(float Value) {
    return { ui_size_percent_of_parent, Value };
}

ui_size UISizeMaxChildren() {
    return { ui_size_max_of_children, 0.0f };
}

ui_size UISizeSumChildren() {
    return { ui_size_sum_of_children, 0.0f };
}

typedef uint32 ui_flags;

enum {
    DRAGGABLE_UI_FLAG = 1 << 2,
};

const int UI_TEXT_LENGTH = 64;
struct ui_element {
    char* Text;
    ui_element* Parent;
    ui_element* Previous;
    ui_element* Next;
    color Color;
    rectangle Rect;
    ui_alignment Alignment[2];
    ui_size Size[2];
    float RelativePosition[2];
    uint32 ID;
    uint32 Index;
    ui_flags Flags;
    bool Hovered;
    bool Clicked;
    bool Dragged;
    bool FirstFrame;
};

const int MAX_UI_ELEMENTS = 64;
struct ui_hierarchy {
    ui_element Elements[MAX_UI_ELEMENTS];
    ui_element* Parent;
    ui_element* First;
    ui_element* Current;
    ui_element* Last;
    uint32 Count;
};

struct ui_context {
    ui_hierarchy Tree;
    render_group* Group;
    game_input* Input;
    std::vector<uint32> IDStack;
    uint32 CurrentIndex;
};

static ui_context UI;

void UISizeText(char * Text, int Points, ui_size* Sizes) {
    rectangle Rect;
    GetTextWidthAndHeight(Text, GetAsset(UI.Group->Assets, Font_Menlo_Regular_ID), Points, &Rect.Width, &Rect.Height);
    Sizes[axis_x].Type = ui_size_text;
    Sizes[axis_x].Value = Rect.Width;
    Sizes[axis_y].Type = ui_size_text;
    Sizes[axis_y].Value = Rect.Height;
}

void PushID(uint32 ID) {
    UI.IDStack.push_back(ID);
    UI.CurrentIndex = 0;
}

void PushID(char* String) {
    PushID(Hash(String));
}

void PopID() {
    UI.IDStack.pop_back();
    UI.CurrentIndex = 0;
}

uint32 GetID(char* String) {
    uint32 i = 0;
    bool UseIndex = false;
    while (String[i]) {
        if (String[i] == '#' && String[i+1] == '#') {
            i = UI.CurrentIndex++;
            UseIndex = true;
            break;
        }
        i++;
    }

    uint32 StackHash = 2166136261u;
    for (uint32 ID : UI.IDStack) {
        StackHash ^= ID;
        StackHash *= 16777619u;
    }
    uint32 TextHash = Hash(String);
    StackHash ^= TextHash;
    StackHash *= 16777619u;
    if (UseIndex) {
        StackHash ^= i;
        StackHash *= 16777619u;
    }
    return StackHash;
}

void PushParent(ui_element* Parent) {
    PushID(Parent->ID);
    Parent->Parent = UI.Tree.Parent;
    UI.Tree.Parent = Parent;
}

void PopParent() {
    if (UI.Tree.Parent == NULL) Raise("No parent in the stack.");
    UI.Tree.Parent = UI.Tree.Parent->Parent;
    PopID();
}

ui_element* NewUIElement(uint32 ID) {
    if (UI.Tree.Count >= MAX_UI_ELEMENTS) Raise("UI Elements Tree is FULL!!");
    ui_element* Result = &UI.Tree.Elements[UI.Tree.Count++];
    Result->ID = ID;
    Result->Index = UI.CurrentIndex;
    Result->FirstFrame = true;
    return Result;
}

ui_element* PushUIElement(
    char* Text,
    ui_size SizeX,
    ui_size SizeY,
    ui_alignment AlignmentX,
    ui_alignment AlignmentY,
    color Color = White
) {
    bool Found = false;
    uint32 ID = GetID(Text);
    ui_element* Element = 0;
    for (int i = 0; i < MAX_UI_ELEMENTS; i++) {
        Element = &UI.Tree.Elements[i];
        if (ID == Element->ID) {
            Found = true;
            Element->FirstFrame = false;
            break;
        }
    }
    if (!Found) Element = NewUIElement(ID);
    if (UI.Tree.First == NULL) UI.Tree.First = Element;

    Element->Parent = UI.Tree.Parent;
    Element->Size[axis_x] = SizeX;
    Element->Size[axis_y] = SizeY;

    ui_size Sizes[2] = { SizeX, SizeY };
    float* RectSizes[2] = { &Element->Rect.Width, &Element->Rect.Height };
    for (int i = 0; i < 2; i++) {
        switch(Sizes[i].Type) {
            case ui_size_null:
            case ui_size_percent_of_parent:
            case ui_size_sum_of_children:
            case ui_size_max_of_children: {
                Element->Size[i].Value = 0.0f;
            } break;
            case ui_size_text:
            case ui_size_pixels: {
                Element->Size[i].Value = Sizes[i].Value;
                *RectSizes[i] = Sizes[i].Value;
            } break;

            default: Raise("Invalid size type");
        }
    }

    Element->Text = Text;
    Element->Color = Color;
    Element->Alignment[axis_x] = AlignmentX;
    Element->Alignment[axis_y] = AlignmentY;
    Element->Hovered = IsIn(Element->Rect, UI.Input->Mouse.Cursor);
    Element->Clicked = Element->Hovered && UI.Input->Mouse.LeftClick.JustPressed;

    if (UI.Tree.Current) UI.Tree.Current->Next = Element;
    Element->Previous = UI.Tree.Current;
    UI.Tree.Current = Element;

    return Element;
}

void ComputeLayout() {
    ui_element* Element = UI.Tree.First;

    if (Element == NULL) return;
    // Computing width and height
    while (Element) {
        for (int i = 0; i < 2; i++) {
            switch(Element->Size[i].Type) {
                case ui_size_null:
                case ui_size_pixels:
                case ui_size_text: {
                    if (i == axis_x) Element->Rect.Width = Element->Size[i].Value;
                    else Element->Rect.Height = Element->Size[i].Value;
                } break;
                case ui_size_percent_of_parent: {
                    ui_element* Parent = Element->Parent;
                    if (Parent->Size[i].Type != ui_size_max_of_children && Parent->Size[i].Type != ui_size_sum_of_children) {
                        if (i == axis_x) Element->Rect.Width  = Element->Size[i].Value * Parent->Size[i].Value;
                        else             Element->Rect.Height = Element->Size[i].Value * Parent->Size[i].Value;
                    }
                } break;
                case ui_size_max_of_children:
                case ui_size_sum_of_children: break;
                default: Raise("Invalid size type.");
            }
        }

        if (Element->Next == NULL) break;
        Element = Element->Next;
    }

    Assert(Element == UI.Tree.Last);
    while (Element) {
        ui_element* Parent = Element->Parent;
        if (Parent) {
            for (int i = 0; i < 2; i++) {
                if (Parent->Size[i].Type == ui_size_max_of_children) {
                    float ChildValue = Element->Size[i].Value;
                    if (ChildValue > Parent->Size[i].Value) Parent->Size[i].Value = ChildValue;
                }
                else if (Parent->Size[i].Type == ui_size_sum_of_children) {
                    Parent->Size[i].Value += Element->Size[i].Value;
                }
            }
        }
        
        if (Element->Previous == NULL) break;
        Element = Element->Previous;
    }

    Assert(Element == UI.Tree.First);
    while(Element) {
        Element->Rect.Width = Element->Size[axis_x].Value;
        Element->Rect.Height = Element->Size[axis_y].Value;
        Element = Element->Next;
    }
}

void BeginContext(game_memory* Memory, game_input* Input) {
    UI.Group = &Memory->RenderGroup;
    UI.Input = Input;
}

struct UIMenu {
    ui_element* Element;
    float Margins[2];
    ui_axis StackAxis;
    ui_axis NoStackAxis;
    float Alpha;

    UIMenu(
        char* Text, 
        ui_axis Stack = axis_y, 
        ui_alignment AlignmentX = ui_alignment_center, 
        ui_alignment AlignmentY = ui_alignment_center,
        float MarginX = 10.0f,
        float MarginY = 10.0f,
        float Opacity = 1.0f
    ) {
        ui_size SizeX, SizeY;
        StackAxis = Stack;
        if (Stack == axis_y) {
            NoStackAxis = axis_x;
            SizeX = UISizeMaxChildren();
            SizeY = UISizeSumChildren();
        }
        else {
            NoStackAxis = axis_y;
            SizeX = UISizeSumChildren();
            SizeY = UISizeMaxChildren();
        }

        Margins[axis_x] = MarginX;
        Margins[axis_y] = MarginY;
        Alpha = Opacity;
        
        Element = PushUIElement(
            Text, 
            SizeX, SizeY,
            AlignmentX, AlignmentY,
            Color(Black, 0.7f * Opacity)
        );
        PushParent(Element);
    }

    ~UIMenu() {
        PopParent();

        // Compute menu size from children
        float StackMargin = Margins[StackAxis];
        float StackSize = StackMargin;
        float NoStackSize = 0;
        ui_element* Child = Element->Next;
        while (Child->Parent == Element) {
            StackSize += Child->Size[StackAxis].Value + StackMargin;
            float ChildValue = Child->Size[NoStackAxis].Value;
            NoStackSize = max(ChildValue, NoStackSize);
            Child = Child->Next;
            if (Child == NULL) break;
        }
        NoStackSize += 2.0f * Margins[NoStackAxis];
        float MenuSize[2];
        MenuSize[StackAxis] = StackSize;
        MenuSize[NoStackAxis] = NoStackSize;

        Element->Rect.Width = MenuSize[axis_x];
        Element->Rect.Height = MenuSize[axis_y];

        // Compute menu position
        Element->Rect.Left = Align(Element->Alignment[axis_x], UI.Group->Width, MenuSize[axis_x], 0);
        Element->Rect.Top  = Align(Element->Alignment[axis_y], UI.Group->Height, MenuSize[axis_y], 0);
        
        // Compute children position
        float NextValue = Margins[StackAxis];
        Child = Element->Next;
        while (Child->Parent == Element) {
            Child->RelativePosition[StackAxis] = NextValue;
            Child->RelativePosition[NoStackAxis] = Align(
                Child->Alignment[NoStackAxis], 
                MenuSize[NoStackAxis], 
                Child->Size[NoStackAxis].Value, 
                Margins[NoStackAxis]
            );

            Child->Rect.Left = Element->Rect.Left + Child->RelativePosition[axis_x];
            Child->Rect.Top  = Element->Rect.Top + Child->RelativePosition[axis_y];

            if (Child->Size[axis_x].Type == ui_size_percent_of_parent) {
                Child->Rect.Width = Child->Size[axis_x].Value * Element->Rect.Width;
            }
            if (Child->Size[axis_y].Type == ui_size_percent_of_parent) {
                Child->Rect.Height = Child->Size[axis_y].Value * Element->Rect.Height;
            }

            NextValue += Margins[StackAxis] + Child->Size[StackAxis].Value;

            Child = Child->Next;
            if (Child == NULL) break;
        }

        // Render rect
        PushRect(UI.Group, Element->Rect, Element->Color, SORT_ORDER_DEBUG_OVERLAY - 1.0f);
    }
};

void UIText(
    char* Text, 
    ui_alignment AlignmentX = ui_alignment_center, ui_alignment AlignmentY = ui_alignment_center, 
    color Color = White,
    int Points = 10
) {
    ui_size Sizes[2];
    UISizeText(Text, Points, Sizes);
    ui_element* Element = PushUIElement(
        Text, 
        Sizes[axis_x], Sizes[axis_y], 
        AlignmentX, AlignmentY, 
        Color
    );
    rectangle Rect = Element->Rect;

    if (!Element->FirstFrame) {
        game_font_id FontID = Font_Menlo_Regular_ID;
        game_font* Font = GetAsset(UI.Group->Assets, FontID);
        PushText(UI.Group, V2(Rect.Left, Rect.Top + GetTextHeight(Font, Points)), FontID, Text, Color, Points);
    }

}

void UIDebugBool(char* Text, bool Value, float Alpha = 1.0f) {
    char* BoolText = Value ? "true " : "false";

    char Buffer[128];
    sprintf_s(Buffer, "%s: %s", Text, BoolText);
    int Points = 10;
    ui_size Sizes[2];
    UISizeText(Buffer, Points, Sizes);
    ui_element* Element = PushUIElement(
        Text,
        Sizes[axis_x], Sizes[axis_y], 
        ui_alignment_min, ui_alignment_center, 
        White
    );
    rectangle Rect = Element->Rect;

    if (!Element->FirstFrame) {
        game_font_id FontID = Font_Menlo_Regular_ID;
        game_font* Font = GetAsset(UI.Group->Assets, FontID);
        sprintf_s(Buffer, "%s: ", Text);
        PushText(UI.Group, V2(Rect.Left, Rect.Top + GetTextHeight(Font, Points)), FontID, Buffer, Color(White, Alpha), Points);

        color BoolColor = Value ? Cyan : Red;
        UISizeText(Buffer, Points, Sizes);
        PushText(
            UI.Group, 
            V2(Rect.Left + Sizes[axis_x].Value, Rect.Top + GetTextHeight(Font, Points)), 
            FontID, 
            BoolText, 
            Color(BoolColor, Alpha), 
            Points
        );
    }
}

void UIDebugFloat(char* Text, float Value, color Color = White) {
    int Points = 10;
    ui_size Sizes[2];
    UISizeText(Text, Points, Sizes);
    ui_element* Element = PushUIElement(
        Text, 
        Sizes[axis_x], Sizes[axis_y], 
        ui_alignment_min, ui_alignment_min, 
        Color
    );
    rectangle Rect = Element->Rect;

    if (!Element->FirstFrame) {
        game_font_id FontID = Font_Menlo_Regular_ID;
        game_font* Font = GetAsset(UI.Group->Assets, FontID);
        char Buffer[128];
        sprintf_s(Buffer, Text, Value);
        PushText(UI.Group, V2(Rect.Left, Rect.Top + GetTextHeight(Font, Points)), FontID, Buffer, Color, Points);
    }
}

void UIDebugInt(char* Text, int Value, color Color = White) {
    int Points = 10;
    ui_size Sizes[2];
    UISizeText(Text, Points, Sizes);
    ui_element* Element = PushUIElement(
        Text, 
        Sizes[axis_x], Sizes[axis_y], 
        ui_alignment_min, ui_alignment_min, 
        Color
    );
    rectangle Rect = Element->Rect;

    if (!Element->FirstFrame) {
        game_font_id FontID = Font_Menlo_Regular_ID;
        game_font* Font = GetAsset(UI.Group->Assets, FontID);
        char Buffer[128];
        sprintf_s(Buffer, Text, Value);
        PushText(UI.Group, V2(Rect.Left, Rect.Top + GetTextHeight(Font, Points)), FontID, Buffer, Color, Points);
    }
}

void UIDebugInt(char* Text, int Value1, int Value2, color Color = White) {
    int Points = 10;
    ui_size Sizes[2];
    UISizeText(Text, Points, Sizes);
    ui_element* Element = PushUIElement(
        Text, 
        Sizes[axis_x], Sizes[axis_y], 
        ui_alignment_min, ui_alignment_min, 
        Color
    );
    rectangle Rect = Element->Rect;

    if (!Element->FirstFrame) {
        game_font_id FontID = Font_Menlo_Regular_ID;
        game_font* Font = GetAsset(UI.Group->Assets, FontID);
        char Buffer[128];
        sprintf_s(Buffer, Text, Value1, Value2);
        PushText(UI.Group, V2(Rect.Left, Rect.Top + GetTextHeight(Font, Points)), FontID, Buffer, Color, Points);
    }
}

bool UIDropdown(char* Text, bool& Control, color Color = White) {
    int Points = 10;
    ui_size Sizes[2];
    UISizeText(Text, Points, Sizes);
    ui_element* Element = PushUIElement(
        Text, 
        Sizes[axis_x], Sizes[axis_y], 
        ui_alignment_min, ui_alignment_min, 
        Color
    );

    bool Hovered = IsIn(Element->Rect, UI.Input->Mouse.Cursor);
    if (Hovered) {
        Element->Color = Yellow;
        Element->Color.Alpha = Color.Alpha;
    }

    bool Clicked = Hovered && UI.Input->Mouse.LeftClick.JustPressed;
    if (Clicked) {
        Control = !Control;
    }

    if (!Element->FirstFrame) {
        game_font_id FontID = Font_Menlo_Regular_ID;
        game_font* Font = GetAsset(UI.Group->Assets, FontID);
        PushText(UI.Group, V2(Element->Rect.Left, Element->Rect.Top + GetTextHeight(Font, Points)), FontID, Text, Element->Color, Points);
    }
    return Control;
}

bool UIButton(char* Text) {
    int Points = 10;
    ui_size Sizes[2];
    UISizeText(Text, Points, Sizes);
    ui_element* Element = PushUIElement(
        Text, 
        Sizes[axis_x], Sizes[axis_y], 
        ui_alignment_center, ui_alignment_center, 
        White
    );

    if (!Element->FirstFrame) {
        color Color = Element->Hovered ? Yellow : White;
        game_font_id FontID = Font_Menlo_Regular_ID;
        game_font* Font = GetAsset(UI.Group->Assets, FontID);
        PushText(UI.Group, V2(Element->Rect.Left, Element->Rect.Top + GetTextHeight(Font, Points)), FontID, Text, Color, Points);
    }
    return Element->Clicked;
}

void UIDebugFillbar(char* Text, float Percent, color C = Red) {
    TIMED_BLOCK;
    ui_size Sizes[2] = { UISizePixels(350.0f), UISizePixels(20.0f) };
    ui_element* Element = PushUIElement(
        Text, 
        Sizes[axis_x], Sizes[axis_y],
        ui_alignment_min, ui_alignment_min,
        C
    );
    
    v2 Position = V2(Element->Rect.Left, Element->Rect.Top);
    rectangle Rect = Element->Rect;

    if (!Element->FirstFrame) {
        PushFillBar(UI.Group, Rect, C, Percent);
        PushText(UI.Group, Position + V2(5.0f, 15.0f), Font_Menlo_Regular_ID, Text, White, 8);
        
        char Buffer[8];
        sprintf_s(Buffer, "%.2f%%", Percent * 100.0f);
        ui_size TextSizes[2];
        UISizeText(Buffer, 8, TextSizes);
        PushText(UI.Group, Position + V2(350.0f - TextSizes[axis_x].Value - 5.0f, 15.0f), Font_Menlo_Regular_ID, Buffer, White, 8);
    }
}

void UIDebugFillbar(char* Text, int Used, int Max, color C = Red) {
    TIMED_BLOCK;
    ui_size Sizes[2] = { UISizePixels(350.0f), UISizePixels(20.0f) };
    ui_element* Element = PushUIElement(
        Text, 
        Sizes[axis_x], Sizes[axis_y],
        ui_alignment_min, ui_alignment_min,
        C
    );
    
    v2 Position = V2(Element->Rect.Left, Element->Rect.Top);
    rectangle Rect = Element->Rect;

    if (!Element->FirstFrame) {
        float Percent = (float)Used / (float)Max;
        PushFillBar(UI.Group, Rect, C, Percent);
        PushText(UI.Group, Position + V2(5.0f, 15.0f), Font_Menlo_Regular_ID, Text, White, 8);
        
        char Buffer[8];
        sprintf_s(Buffer, "%d/%d", Used, Max);
        ui_size TextSizes[2];
        UISizeText(Buffer, 8, TextSizes);
        PushText(UI.Group, Position + V2(350.0f - TextSizes[axis_x].Value - 5.0f, 15.0f), Font_Menlo_Regular_ID, Buffer, White, 8);
    }
}

void UIDebugArena(char* Text, memory_arena Arena, color C = Red) {
    UIDebugFillbar(Text, (float)Arena.Used / (float)Arena.Size, C);
}

void UpdateUI(
    game_memory* Memory,
    game_input* Input
) {
    render_group* Group = &Memory->RenderGroup;
    game_state* pGameState = (game_state*)Memory->PermanentStorage;
    game_entity_state* EntityState = &pGameState->Entities;
    float Time = pGameState->Time;
    debug_info DebugInfo = Memory->DebugInfo;

    BeginContext(Memory, Input);

    UI.Tree.First = 0;

    // Main menu
    static bool ShowMainMenu = false;
    bool MainMenuInput = Input->Mode == Keyboard && Input->Keyboard.Escape.JustPressed ||
                         Input->Mode == Controller && Input->Controller.Start.JustPressed;
    if (MainMenuInput) {
        ShowMainMenu = !ShowMainMenu;
    }
    if (ShowMainMenu) {
        UIMenu MainMenu("Main menu", axis_y, ui_alignment_center, ui_alignment_center, 50.0f, 30.0f);
        if (UIButton("Save game")) {
            // TODO: Save game
        }

        if (UIButton("Load game")) {
            // TODO: Load game
        }

        if (UIButton("Settings")) {
            // TODO: Change settings
        }

        if (UIButton("Exit")) {
            pGameState->Exit = true;
        }
    }

    // Debug UI
    static float DebugAlpha = 0.0f;
    if (
        Input->Mode == Keyboard && Input->Keyboard.F1.JustPressed || 
        Input->Mode == Controller && Input->Controller.Back.JustPressed
    ) {
        Group->Debug = !Group->Debug;
        if (!Group->Debug) DebugAlpha = 0.0;
    }
    
    if (Group->Debug) {
        // Handle input
        if (DebugAlpha < 1.0) {
            double x = (pGameState->dt - 1.8) / 1.1;
            DebugAlpha += 0.5 * exp(- x * x);
        }
        else DebugAlpha = 1.0;

        if (Input->Keyboard.N.IsDown && !Input->Keyboard.N.WasDown) Group->DebugNormals = !Group->DebugNormals;
        if (Input->Keyboard.B.IsDown && !Input->Keyboard.B.WasDown) Group->DebugBones = !Group->DebugBones;
        if (Input->Keyboard.C.IsDown && !Input->Keyboard.C.WasDown) Group->DebugColliders = !Group->DebugColliders;
        
        PushDebugGrid(Group, DebugAlpha);
        
        UIMenu DebugMenu("Debug Menu", axis_y, ui_alignment_min, ui_alignment_min, 10.0f, 10.0f, DebugAlpha);

        // Axes
        v2 XAxis = V2(cos(Group->Camera->Angle * Degrees), sin(Group->Camera->Angle * Degrees) * sin(Group->Camera->Pitch * Degrees));
        v2 YAxis = V2(0.0, -cos(Group->Camera->Pitch * Degrees));
        v2 ZAxis = V2(-sin(Group->Camera->Angle * Degrees), sin(Group->Camera->Pitch * Degrees) * cos(Group->Camera->Angle * Degrees));
        v2 AxisOrigin = V2(Group->Width - 0.08 * (float)Group->Height - 10.0, 0.1 * (float)Group->Height);
        PushDebugVector(Group, 0.08 * Group->Height * XAxis, AxisOrigin, Color(Red, DebugAlpha));
        PushDebugVector(Group, 0.08 * Group->Height * YAxis, AxisOrigin, Color(Green, DebugAlpha));
        PushDebugVector(Group, 0.08 * Group->Height * ZAxis, AxisOrigin, Color(Blue, DebugAlpha));

        // Debug camera basis
        // PushDebugVector(Group, Group->Camera.Basis.X, V3(0,0,0), World_Coordinates, Yellow);
        // PushDebugVector(Group, Group->Camera.Basis.Y, V3(0,0,0), World_Coordinates, Magenta);
        // PushDebugVector(Group, Group->Camera.Basis.Z, V3(0,0,0), World_Coordinates, Cyan);

        // Debug Framebuffer
        PushDebugFramebuffer(Group, Target_Postprocessing_Outline);

        static bool DebugArenas = false;
        if (UIDropdown("Arenas", DebugArenas, Color(White, DebugAlpha))) {
            UIDebugArena("Strings Arena", Memory->StringsArena, Color(Red, DebugAlpha));
            UIDebugArena("Transient Arena", Memory->TransientArena, Color(Red, DebugAlpha));
            UIDebugArena("General purpose Arena", Memory->GeneralPurposeArena, Color(Red, DebugAlpha));
            UIDebugArena("Vertex arena (vec3)", Group->VertexBuffer.VertexArena[0], Color(Red, DebugAlpha));
            UIDebugArena("Vertex arena (vec3, vec2)", Group->VertexBuffer.VertexArena[1], Color(Red, DebugAlpha));
            UIDebugArena("Vertex arena (vec3, vec2, vec3)", Group->VertexBuffer.VertexArena[2], Color(Red, DebugAlpha));
            UIDebugArena("Vertex arena (mesh with bones)", Group->VertexBuffer.VertexArena[3], Color(Red, DebugAlpha));
        }

        static bool DebugEntities = false;
        if (UIDropdown("Entities", DebugEntities, Color(White, DebugAlpha))) {
            character* Character = &EntityState->Characters.List[0];
            char* ActionNames[4] = { "Idle", "Walk", "Jump", "Attack" };
            UIText("Character Action:", ui_alignment_min, ui_alignment_center, Color(White, DebugAlpha));
            UIText(ActionNames[Character->Action.ID], ui_alignment_center, ui_alignment_center);
        }
    }

    if (UI.Tree.Current) UI.Tree.Current->Next = 0;
    UI.Tree.Last = UI.Tree.Current;

    UI.Tree.Current = 0;
    UI.Tree.Parent = 0;
}

#endif