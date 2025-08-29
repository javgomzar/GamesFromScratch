#ifndef GAME_UI
#define GAME_UI

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
    Returns the relative offset of a `Contained` length when fitting it inside a `Container` following `Alignment` (center, min or max).
    Optional `Margin` input adds a margin to this computation.
        - Example: `Align(ui_alignment_center, 100.0f, 50.0f) == 25.0f`
*/
float Align(ui_alignment Alignment, float Container, float Contained, float Margin = 0.0f) {
    switch(Alignment) {
        case ui_alignment_center: { return 0.5f * (Container - Contained); } break;
        case ui_alignment_min:    { return Margin; } break;
        case ui_alignment_max:    { return Container - Contained - Margin; } break;
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
    float Min = 0;
    float Max = FLT_MAX;
};

ui_size UISizeNull() {
    return { ui_size_null, 0.0f };
}

ui_size UISizePixels(float Value) {
    return { ui_size_pixels, Value, Value, Value };
}

ui_size UISizePercentParent(float Value, float Min = 0.0f, float Max = FLT_MAX) {
    return { ui_size_percent_of_parent, Value, Min, Max };
}

ui_size UISizeMaxChildren(float Min = 0.0f, float Max = FLT_MAX) {
    return { ui_size_max_of_children, Min, Min, Max };
}

ui_size UISizeSumChildren(float InitialValue = 0.0f, float Max = FLT_MAX) {
    return { ui_size_sum_of_children, InitialValue, InitialValue, Max };
}

typedef uint32 ui_flags;

enum {
    RENDER_TEXT_UI_FLAG      = 1 << 0,
    RENDER_RECT_UI_FLAG      = 1 << 1,
    STACK_CHILDREN_X_UI_FLAG = 1 << 2,
    STACK_CHILDREN_Y_UI_FLAG = 1 << 3,
};

struct ui_element {
    char Name[64];
    ui_element* Parent;
    ui_element* Previous;
    ui_element* Next;
    debug_entry* DebugEntry;

    color Color;
    rectangle Rect;
    float RelativePosition[2];
    float Margins[2];
    float Points;

    uint32 ID;
    uint32 Index;

    ui_size Size[2];
    ui_alignment Alignment[2];
    ui_flags Flags;

    bool Hovered;
    bool Clicked;
    bool Dragged;
    bool Expand;
};

const int MAX_UI_ELEMENTS = 256;
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
    debug_info* DebugInfo;
    std::vector<uint32> IDStack;
    uint32 CurrentIndex;
};

static ui_context UI;

void UISizeText(const char * Text, int Points, ui_size* Sizes) {
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

uint32 GetID(const char* String) {
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
    return Result;
}

ui_element* PushUIElement(
    const char* Name,
    ui_size SizeX,
    ui_size SizeY,
    ui_alignment AlignmentX,
    ui_alignment AlignmentY,
    ui_flags Flags = 0
) {
    bool Found = false;
    uint32 ID = GetID(Name);
    ui_element* Element = 0;
    for (int i = 0; i < MAX_UI_ELEMENTS; i++) {
        Element = &UI.Tree.Elements[i];
        if (ID == Element->ID) {
            Found = true;
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
            {
                Element->Size[i].Value = 0.0f;
            } break;

            case ui_size_sum_of_children:
            case ui_size_max_of_children: break;

            case ui_size_text:
            case ui_size_pixels: {
                Element->Size[i].Value = Sizes[i].Value;
                *RectSizes[i] = Sizes[i].Value;
            } break;

            default: Raise("Invalid size type");
        }
    }

    strcpy_s(Element->Name, Name);
    Element->Color = White;
    Element->Alignment[axis_x] = AlignmentX;
    Element->Alignment[axis_y] = AlignmentY;
    Element->Flags = Flags;
    Element->Hovered = IsIn(Element->Rect, UI.Input->Mouse.Cursor);
    Element->Clicked = Element->Hovered && UI.Input->Mouse.LeftClick.JustPressed;

    if (UI.Tree.Current) UI.Tree.Current->Next = Element;
    Element->Previous = UI.Tree.Current;
    UI.Tree.Current = Element;

    return Element;
}

void BeginContext(game_memory* Memory, game_input* Input) {
    UI.Group = &Memory->RenderGroup;
    UI.Input = Input;
    UI.DebugInfo = &Memory->DebugInfo;
}

void ComputeSizes() {
    ui_element* Element = UI.Tree.First;
    if (Element == NULL) return;

    game_font* Font = GetAsset(UI.Group->Assets, Font_Menlo_Regular_ID);

    // Go from top to bottom computing sizes that don't depend on children
    while (Element) {
        for (int i = 0; i < 2; i++) {
            switch(Element->Size[i].Type) {
                case ui_size_null: {
                    if (i == axis_x) Element->Rect.Width = 0;
                    else             Element->Rect.Height = 0;
                } break;

                case ui_size_pixels:
                case ui_size_text: {
                    if (i == axis_x) Element->Rect.Width = Element->Size[i].Value;
                    else             Element->Rect.Height = Element->Size[i].Value;
                } break;

                case ui_size_percent_of_parent: {
                    ui_element* Parent = Element->Parent;
                    if (Parent->Size[i].Type != ui_size_max_of_children && Parent->Size[i].Type != ui_size_sum_of_children) {
                        if (i == axis_x) Element->Rect.Width  = Element->Size[i].Value * Parent->Size[i].Value - 2.0f * Parent->Margins[i];
                        else             Element->Rect.Height = Element->Size[i].Value * Parent->Size[i].Value - 2.0f * Parent->Margins[i];
                    }
                } break;

                case ui_size_max_of_children: 
                case ui_size_sum_of_children: { 
                    Element->Size[i].Value += Element->Margins[i];
                } break;
                
                default: Raise("Invalid size type.");
            }

            if (Element->Size[i].Value > Element->Size[i].Max) {
                Element->Size[i].Value = Element->Size[i].Max;
            }
            if (Element->Size[i].Value < Element->Size[i].Min) {
                Element->Size[i].Value = Element->Size[i].Min;
            }
        }

        if (Element->Next == NULL) break;
        Element = Element->Next;
    }

    Assert(Element == UI.Tree.Last);

    // Go from bottom to top computing sizes that depend on children
    while (Element) {
        ui_element* Parent = Element->Parent;
        if (Parent) {
            for (int i = 0; i < 2; i++) {
                if (Parent->Size[i].Type == ui_size_max_of_children) {
                    float ChildValue = Element->Size[i].Value + 2.0f * Parent->Margins[i];
                    if (ChildValue > Parent->Size[i].Value) Parent->Size[i].Value = ChildValue;
                }
                else if (Parent->Size[i].Type == ui_size_sum_of_children) {
                    Parent->Size[i].Value += Element->Size[i].Value + Parent->Margins[i];
                }
            }
            Parent->Rect.Width = Parent->Size[axis_x].Value;
            Parent->Rect.Height = Parent->Size[axis_y].Value;
        }
        
        if (Element->Previous == NULL) break;
        Element = Element->Previous;
    }

    Assert(Element == UI.Tree.First);
    
    // Go back from top to bottom to compute parent dependent sizes
    float GroupSizes[2] = { (float)UI.Group->Width, (float)UI.Group->Height };
    while (Element) {
        for (int i = 0; i < 2; i++) {
            if (Element->Size[i].Type == ui_size_percent_of_parent) {
                ui_element* Parent = Element->Parent;
                float ParentSize = Parent ? Parent->Size[i].Value : GroupSizes[i];
                if (i == axis_x) Element->Rect.Width  = Element->Size[i].Value * ParentSize - 2.0f * Parent->Margins[i];
                else             Element->Rect.Height = Element->Size[i].Value * ParentSize - 2.0f * Parent->Margins[i];
            }
        }
        
        if (Element->Next == NULL) break;
        Element = Element->Next;
    }
}

void ComputeLayout() {
    // Compute relative positions
    ui_element* Element = UI.Tree.First;
    if (Element == NULL) return;

    while(Element) {
        float ParentWidth = UI.Group->Width;
        float ParentHeight = UI.Group->Height;
        float MarginX = 0.0f, MarginY = 0.0f;
        if (Element->Parent != NULL) {
            ParentWidth = Element->Parent->Rect.Width;
            ParentHeight = Element->Parent->Rect.Height;
            MarginX = Element->Parent->Margins[axis_x];
            MarginY = Element->Parent->Margins[axis_y];
        }

        if (Element->Parent == NULL) {
            Element->Rect.Left = Align(Element->Alignment[axis_x], ParentWidth, Element->Rect.Width, MarginX) + Element->RelativePosition[axis_x];
            Element->Rect.Top = Align(Element->Alignment[axis_y], ParentHeight, Element->Rect.Height, MarginY) + Element->RelativePosition[axis_y];
        }
        
        // Layout that depends on parents
        if (Element->Flags & (STACK_CHILDREN_X_UI_FLAG | STACK_CHILDREN_Y_UI_FLAG)) {
            ui_axis StackAxis = (Element->Flags & STACK_CHILDREN_X_UI_FLAG) ? axis_x : axis_y;
            ui_axis NoStackAxis = StackAxis == axis_x ? axis_y : axis_x;

            float NextValue = Element->Margins[StackAxis] + Element->Size[StackAxis].Min;
            ui_element* Child = Element->Next;
            while (Child) {
                if (Child->Parent == Element) {
                    Child->RelativePosition[StackAxis] = NextValue;
                    Child->RelativePosition[NoStackAxis] = Align(
                        Child->Alignment[NoStackAxis], 
                        Element->Size[NoStackAxis].Value, 
                        Child->Size[NoStackAxis].Value, 
                        Element->Margins[NoStackAxis]
                    );
                    
                    Child->Rect.Left = Element->Rect.Left + Child->RelativePosition[axis_x];
                    Child->Rect.Top  = Element->Rect.Top + Child->RelativePosition[axis_y];

                    NextValue += Element->Margins[StackAxis] + Child->Size[StackAxis].Value;
                }

                Child = Child->Next;
            };
        }

        Element = Element->Next;
    }
}

void RenderUI() {
    TIMED_BLOCK;
    
    game_font_id FontID = Font_Menlo_Regular_ID;
    game_font* Font = GetAsset(UI.Group->Assets, FontID);

    ui_element* Element = UI.Tree.First;
    while (Element) {
        v2 Position = LeftTop(Element->Rect);
        if (Element->DebugEntry) {
            PushDebugEntry(UI.Group, Element->DebugEntry, Position, Element->Color);
        }
        else {
            if (Element->Flags & RENDER_RECT_UI_FLAG) {
                PushRect(UI.Group, Element->Rect, Element->Color);
            }

            if (Element->Flags & RENDER_TEXT_UI_FLAG) {
                float OffsetHeight = GetCharMaxHeight(Font, Element->Points);
                PushText(
                    UI.Group,
                    Position + V2(0, OffsetHeight), 
                    FontID, 
                    Element->Name,
                    Element->Color,
                    Element->Points
                );
            }
        }

        Element = Element->Next;
    }
}

struct UIMenu {
    UIMenu(
        const char* Text,
        ui_axis Stack = axis_y, 
        ui_alignment AlignmentX = ui_alignment_center, 
        ui_alignment AlignmentY = ui_alignment_center,
        float MarginX = 10.0f,
        float MarginY = 10.0f,
        color C = ChangeAlpha(Black, 0.7f)
    ) {
        ui_axis NoStack = Stack == axis_x ? axis_y : axis_x;
        ui_size Sizes[2];
        Sizes[Stack] = UISizeSumChildren();
        Sizes[NoStack] = UISizeMaxChildren();
        ui_flags Flags = RENDER_RECT_UI_FLAG;
        if (Stack == axis_x) {
            Flags |= STACK_CHILDREN_X_UI_FLAG;
        }
        else {
            Flags |= STACK_CHILDREN_Y_UI_FLAG;
        }
        ui_element* Element = PushUIElement(Text, Sizes[0], Sizes[1], AlignmentX, AlignmentY, Flags);
        Element->Margins[axis_x] = MarginX;
        Element->Margins[axis_y] = MarginY;
        Element->Color = C;
        PushParent(Element);
    }

    ~UIMenu() {
        PopParent();
    }
};

struct _UIDropdown {
    bool Expand;

    _UIDropdown(const char* Text) {
        game_font* Font = GetAsset(UI.Group->Assets, Font_Menlo_Regular_ID);
        int Points = 12;
        float Width = 0, Height = 0;
        GetTextWidthAndHeight(Text, Font, Points, &Width, &Height);
        ui_size Sizes[2] = {
            UISizeMaxChildren(Width),
            UISizeSumChildren(Height)
        };
        
        ui_element* Element = PushUIElement(
            Text, 
            Sizes[axis_x], Sizes[axis_y], 
            ui_alignment_min, ui_alignment_min,
            RENDER_TEXT_UI_FLAG | STACK_CHILDREN_Y_UI_FLAG
        );
        Element->Points = Points;
        Element->Rect.Width = Width;
        Element->Rect.Height = Height;

        bool Hovered = IsIn(Element->Rect, UI.Input->Mouse.Cursor);
        if (Hovered) {
            Element->Color = Yellow;
        }

        if (Hovered && UI.Input->Mouse.LeftClick.JustPressed) {
            Element->Expand = !Element->Expand;
        }

        Expand = Element->Expand;

        PushParent(Element);
    }

    ~_UIDropdown() {
        PopParent();
    }

    operator bool() const { return Expand; }
};

#define UIDropdown(Name) _UIDropdown _##Name(#Name); _##Name

void UIText(
    const char* Text, 
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
        RENDER_TEXT_UI_FLAG
    );
    Element->Color = Color;
    Element->Points = Points;
}

bool UIButton(const char* Text) {
    float Points = 20.0f;
    ui_size Sizes[2];
    UISizeText(Text, Points, Sizes);
    ui_element* Element = PushUIElement(
        Text, 
        Sizes[axis_x], Sizes[axis_y], 
        ui_alignment_center, ui_alignment_center,
        RENDER_TEXT_UI_FLAG
    );
    Element->Points = Points;

    bool Hovered = IsIn(Element->Rect, UI.Input->Mouse.Cursor);
    if (Hovered) {
        Element->Color = Yellow;
    }

    return Element->Clicked;
}

void UIDebugValue(debug_entry* Entry) {
    game_font_id FontID = Font_Menlo_Regular_ID;
    game_font* Font = GetAsset(UI.Group->Assets, FontID);
    ui_size Sizes[2] = {
        {ui_size_pixels, 0},
        {ui_size_pixels, 0},
    };
    UpdateAndSizeDebugEntry(Font, Entry, &Sizes[axis_x].Value, &Sizes[axis_y].Value);

    ui_element* Element = PushUIElement(Entry->Name, Sizes[0], Sizes[1], ui_alignment_min, ui_alignment_center);
    Element->DebugEntry = Entry;
    if (IsStructType(Entry->Type)) {
        Element->Size[axis_x] = UISizeMaxChildren(Sizes[axis_x].Value);
        Element->Size[axis_y] = UISizeSumChildren(Sizes[axis_y].Value);
        Element->Flags = STACK_CHILDREN_Y_UI_FLAG;
        if (Element->Hovered) Element->Color = Yellow;
        if (Entry->Value != NULL && Element->Clicked) {
            Element->Expand = !Element->Expand;
        }
        if (Element->Expand) {
            PushParent(Element);
            // Add members
            bool Found = false;
            for (int i = 0; i < STRUCT_MEMBERS_SIZE; i++) {
                debug_struct_member Member = StructMembers[i];
                if (Member.StructType == Entry->Type) {
                    if (!Found) Found = true;
                    while (Member.StructType == Entry->Type) {
                        uint8* Pointer = (uint8*)Entry->Value + Member.Offset;
                        if (Member.IsPointer) {
                            debug_entry* ChildEntry = _AddDebugEntry(
                                UI.DebugInfo, 
                                Member.Name, 
                                Member.MemberType, 
                                Member.Size, 
                                Pointer ? *(void**)Pointer : NULL, 
                                Entry->Editable, 
                                Entry
                            );
                            UIDebugValue(ChildEntry);
                        }
                        else if (Member.ArraySize > 0) {
                            for (int j = 0; j < Member.ArraySize; j++) {
                                debug_entry* ChildEntry = _AddDebugEntry(
                                    UI.DebugInfo, 
                                    Member.Name, 
                                    Member.MemberType, 
                                    Member.Size, 
                                    Pointer, 
                                    Entry->Editable, 
                                    Entry
                                );
                                Pointer += Member.Size;
                                UIDebugValue(ChildEntry);
                            }
                        }
                        else {
                            debug_entry* ChildEntry = _AddDebugEntry(
                                UI.DebugInfo, 
                                Member.Name, 
                                Member.MemberType, 
                                Member.Size, 
                                (void*)Pointer, 
                                Entry->Editable, 
                                Entry
                            );
                            UIDebugValue(ChildEntry);
                        }
                        i++;
                        if (i < STRUCT_MEMBERS_SIZE) Member = StructMembers[i];
                        else break;
                    }
                    break;
                }
            }
            PopParent();
        }
    }
}

void UpdateUI(
    game_memory* Memory,
    game_input* Input
) {
    render_group* Group = &Memory->RenderGroup;
    game_state* pGameState = (game_state*)Memory->Permanent.Base;
    game_entity_state* EntityState = &pGameState->Entities;
    float Time = pGameState->Time;
    debug_info* DebugInfo = &Memory->DebugInfo;

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
        UIMenu MainMenu = UIMenu("Main menu", axis_y, ui_alignment_center, ui_alignment_center, 50.0f, 20.0f);

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

        if (Input->Keyboard.N.JustPressed) Group->DebugNormals = !Group->DebugNormals;
        if (Input->Keyboard.B.JustPressed) Group->DebugBones = !Group->DebugBones;
        if (Input->Keyboard.C.JustPressed) Group->DebugColliders = !Group->DebugColliders;
        
        PushDebugGrid(Group, DebugAlpha);

        // Axes
        v2 XAxis = V2(cos(Group->Camera->Angle * Degrees), sin(Group->Camera->Angle * Degrees) * sin(Group->Camera->Pitch * Degrees));
        v2 YAxis = V2(0.0, -cos(Group->Camera->Pitch * Degrees));
        v2 ZAxis = V2(-sin(Group->Camera->Angle * Degrees), sin(Group->Camera->Pitch * Degrees) * cos(Group->Camera->Angle * Degrees));
        v2 AxisOrigin = V2(Group->Width - 0.08 * (float)Group->Height - 10.0, 0.1 * (float)Group->Height);
        PushDebugVector(Group, 0.08 * Group->Height * XAxis, AxisOrigin, ChangeAlpha(Red, DebugAlpha));
        PushDebugVector(Group, 0.08 * Group->Height * YAxis, AxisOrigin, ChangeAlpha(Green, DebugAlpha));
        PushDebugVector(Group, 0.08 * Group->Height * ZAxis, AxisOrigin, ChangeAlpha(Blue, DebugAlpha));

        // Debug camera basis
        // PushDebugVector(Group, Group->Camera.Basis.X, V3(0,0,0), World_Coordinates, Yellow);
        // PushDebugVector(Group, Group->Camera.Basis.Y, V3(0,0,0), World_Coordinates, Magenta);
        // PushDebugVector(Group, Group->Camera.Basis.Z, V3(0,0,0), World_Coordinates, Cyan);
        
        UIMenu DebugMenu = UIMenu("Debug Menu", axis_y, ui_alignment_min, ui_alignment_min, 5.0f, 0.0f);

        int i = 0;
        int nEntries = DebugInfo->nEntries;
        for (; i < nEntries; i++) {
            debug_entry* Entry = &DebugInfo->Entries[i];
            UIDebugValue(Entry);
        }

        if (UIDropdown(Arenas)) {
            DEBUG_VALUE(Memory->Transient, memory_arena);
            DEBUG_VALUE(Memory->Permanent, memory_arena);

            memory_arena* VertexArena = Group->VertexBuffer.Vertices;
            DEBUG_VALUE(VertexArena[vertex_layout_vec2_id], memory_arena);
            DEBUG_VALUE(VertexArena[vertex_layout_vec2_vec2_id], memory_arena);
            DEBUG_VALUE(VertexArena[vertex_layout_vec3_id], memory_arena);
            DEBUG_VALUE(VertexArena[vertex_layout_vec3_vec2_id], memory_arena);
            DEBUG_VALUE(VertexArena[vertex_layout_vec3_vec2_vec3_id], memory_arena);
            DEBUG_VALUE(VertexArena[vertex_layout_bones_id], memory_arena);
            nEntries = DebugInfo->nEntries;
            for (; i < nEntries; i++) {
                debug_entry* Entry = &DebugInfo->Entries[i];
                UIDebugValue(Entry);
            }
        }

        if (UIDropdown(Entities)) {
            game_entity* Entities = EntityState->Entities.List;
            DEBUG_ARRAY(Entities, EntityState->Entities.Count, game_entity);
            nEntries = DebugInfo->nEntries;
            for (; i < nEntries; i++) {
                debug_entry* Entry = &DebugInfo->Entries[i];
                UIDebugValue(Entry);
            }
        }
        
        // Debug Framebuffer
        PushDebugFramebuffer(Group, Target_Postprocessing_Outline);
    }

    if (UI.Tree.Current) UI.Tree.Current->Next = 0;
    UI.Tree.Last = UI.Tree.Current;

    ComputeSizes();
    ComputeLayout();
    RenderUI();

    UI.Tree.Current = 0;
    UI.Tree.Parent = 0;
}

#endif