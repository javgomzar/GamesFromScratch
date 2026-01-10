#ifndef GAME_UI
#define GAME_UI

enum ui_axis {
    axis_x,
    axis_y
};

ui_axis Opposite(ui_axis Axis) {
    return Axis == axis_x ? axis_y : axis_x;
}

enum ui_alignment {
    ui_alignment_free,
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

// This size will be skipped when computing children dependent sizes and layout
ui_size UISizeNull(float Value = 0.0f) {
    return { ui_size_null, Value };
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
    game_font_id Font;
    rectangle Rect;
    float RelativePosition[2];
    float Margins[2];
    float Points;
    float Scroll;

    uint32 ID;
    uint32 Index;

    ui_size Size[2];
    ui_alignment Alignment[2];
    ui_flags Flags;

    bool Hovered;
    bool Clicked;
    bool Dragged;
    bool Expanded;
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
    stack<uint32> IDStack;
    uint32 CurrentIndex;
};

static ui_context UI = {};

void UISizeText(const char * Text, int Points, ui_size* Sizes) {
    rectangle Rect;
    GetTextWidthAndHeight(Text, GetAsset(UI.Group->Assets, Font_Menlo_Regular_ID), Points, &Rect.Width, &Rect.Height);
    Sizes[axis_x].Type = ui_size_text;
    Sizes[axis_x].Value = Rect.Width;
    Sizes[axis_y].Type = ui_size_text;
    Sizes[axis_y].Value = Rect.Height;
}

void PushID(uint32 ID) {
    UI.IDStack.Push(ID);
    UI.CurrentIndex = 0;
}

void PushID(char* String) {
    PushID(Hash(String));
}

void PopID() {
    UI.IDStack.Pop();
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
    for (int j = 0; j < UI.IDStack.n; j++) {
        uint32 ID = UI.IDStack[j];
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
    *Result = {};
    Result->ID = ID;
    Result->Index = UI.CurrentIndex;
    Result->Scroll = 0;
    Result->Font = Font_Menlo_Regular_ID;
    Result->Color = White;
    return Result;
}

struct ui_element_options {
    ui_alignment AlignmentX = ui_alignment_free;
    ui_alignment AlignmentY = ui_alignment_free;
    color Color             = White;
    ui_flags Flags          = 0;
    game_font_id Font       = Font_Menlo_Regular_ID;
    float MarginX           = 0.0f;
    float MarginY           = 0.0f;
    float Points            = 20.0f;
    ui_size SizeX           = UISizeNull();
    ui_size SizeY           = UISizeNull();
    ui_axis Stack           = axis_y;
};

ui_element* _PushUIElement(
    const char* Name,
    ui_element_options Options = {}
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
    ui_size Sizes[2] = { Options.SizeX, Options.SizeY };
    Element->Size[axis_x] = Sizes[axis_x];
    Element->Size[axis_y] = Sizes[axis_y];
    Element->Margins[axis_x] = Options.MarginX;
    Element->Margins[axis_y] = Options.MarginY;

    float* RectSizes[2] = { &Element->Rect.Width, &Element->Rect.Height };
    for (int i = 0; i < 2; i++) {
        switch(Sizes[i].Type) {
            case ui_size_null:
            case ui_size_percent_of_parent:
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
    Element->Color = Options.Color;
    Element->Font = Options.Font;
    Element->Points = Options.Points;
    Element->Alignment[axis_x] = Options.AlignmentX;
    Element->Alignment[axis_y] = Options.AlignmentY;
    Element->Flags = Options.Flags;
    Element->Hovered = IsIn(Element->Rect, UI.Input->Mouse.Cursor);
    Element->Clicked = Element->Hovered && UI.Input->Mouse.LeftClick.JustPressed;

    if (Element->Clicked) {
        Element->Dragged = true;
    }

    if (Element->Dragged) {
        if (UI.Input->Mouse.LeftClick.JustLifted) {
            Element->Dragged = false;
        }
    }

    if (UI.Tree.Current) UI.Tree.Current->Next = Element;
    Element->Previous = UI.Tree.Current;
    UI.Tree.Current = Element;

    return Element;
}

#define PushUIElement(Name, ...) _PushUIElement(Name, { __VA_ARGS__ })

void BeginContext(game_memory* Memory, game_input* Input) {
    UI.Group = &Memory->RenderGroup;
    UI.Input = Input;
    UI.DebugInfo = &Memory->DebugInfo;
}

void ComputeSizes() {
    ui_element* Element = UI.Tree.First;
    if (Element == NULL) return;

    game_font* Font = GetAsset(UI.Group->Assets, Font_Menlo_Regular_ID);

    float GroupSizes[2] = { (float)UI.Group->Width, (float)UI.Group->Height };

    // Go from top to bottom computing sizes that don't depend on children
    while (Element) {
        for (int i = 0; i < 2; i++) {
            switch(Element->Size[i].Type) {
                case ui_size_null:
                case ui_size_pixels:
                case ui_size_text: {
                    if (i == axis_x) Element->Rect.Width = Element->Size[i].Value;
                    else             Element->Rect.Height = Element->Size[i].Value;
                } break;

                case ui_size_percent_of_parent: {
                    ui_element* Parent = Element->Parent;
                    float ParentSize = Parent ? Parent->Size[i].Value : GroupSizes[i];
                    float ParentMargin = Parent ? Parent->Margins[i] : 0;

                    if (i == axis_x) Element->Rect.Width  = Element->Size[i].Value * ParentSize - 2.0f * ParentMargin;
                    else             Element->Rect.Height = Element->Size[i].Value * ParentSize - 2.0f * ParentMargin;
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
                if (Element->Size[i].Type != ui_size_null) {
                    if (Parent->Size[i].Type == ui_size_max_of_children) {
                        float ChildValue = Element->Size[i].Value + 2.0f * Parent->Margins[i];
                        if (ChildValue > Parent->Size[i].Value) Parent->Size[i].Value = ChildValue;
                    }
                    else if (Parent->Size[i].Type == ui_size_sum_of_children) {
                        Parent->Size[i].Value += Element->Size[i].Value + Parent->Margins[i];
                    }
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
    while (Element) {
        for (int i = 0; i < 2; i++) {
            float Result = 0;
            if (Element->Size[i].Type == ui_size_percent_of_parent) {
                ui_element* Parent = Element->Parent;
                float ParentSize = Parent ? Parent->Size[i].Value : GroupSizes[i];
                float ParentMargin = Parent ? Parent->Margins[i] : 0;
                Result = Element->Size[i].Value * ParentSize - 2.0f * ParentMargin;

                if (i == axis_x) Element->Rect.Width  = Result;
                else             Element->Rect.Height = Result;
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
        // Some elements lay out their children
        if (Element->Flags & (STACK_CHILDREN_X_UI_FLAG | STACK_CHILDREN_Y_UI_FLAG)) {
            ui_axis StackAxis = (Element->Flags & STACK_CHILDREN_X_UI_FLAG) ? axis_x : axis_y;

            float NextValue = Element->Margins[StackAxis] + Element->Size[StackAxis].Min;
            ui_element* Child = Element->Next;
            while (Child) {
                if (Child->Parent == Element) {
                    if (Child->Size[StackAxis].Type != ui_size_null) {
                        Child->RelativePosition[StackAxis] = NextValue;
                        NextValue += Element->Margins[StackAxis] + Child->Size[StackAxis].Value;
                    }
                }

                Child = Child->Next;
            };
        }

        // All elements have a relative position to their parent
        float ParentSizes[2]    = { (float)UI.Group->Width, (float)UI.Group->Height };
        float ParentMargins[2]  = { 0, 0 };
        float ParentPosition[2] = { 0, 0 };

        bool HasParent = Element->Parent != NULL;
        if (HasParent) {
            ParentSizes[axis_x] = Element->Parent->Rect.Width;
            ParentSizes[axis_y] = Element->Parent->Rect.Height;

            ParentMargins[axis_x] = Element->Parent->Margins[axis_x];
            ParentMargins[axis_y] = Element->Parent->Margins[axis_y];

            ParentPosition[axis_x] = Element->Parent->Rect.Left;
            ParentPosition[axis_y] = Element->Parent->Rect.Top;
        }

        float* ElementRectPos  = &Element->Rect.Left;
        float* ElementRectSize = &Element->Rect.Width;
        for (int i = 0; i < 2; i++) {
            ElementRectPos[i] = ParentPosition[i] + Element->RelativePosition[i];
            ui_alignment Alignment = Element->Alignment[i];
            if (Alignment != ui_alignment_free)
                ElementRectPos[i] += Align(Alignment, ParentSizes[i], ElementRectSize[i], ParentMargins[i]);
        }

        // Some children control their parents

        Element = Element->Next;
    }
}

void RenderUI() {
    rectangle Screen = {0, 0, (float)UI.Group->Width, (float)UI.Group->Height};

    ui_element* Element = UI.Tree.First;
    while (Element) {
        v2 Position = LeftTop(Element->Rect);
        if (Intersect(Screen, Element->Rect)) {
            if (Element->DebugEntry) {
                PushDebugEntry(UI.Group, Element->DebugEntry, Position, Element->Color);
            }
            else {
                if (Element->Flags & RENDER_RECT_UI_FLAG) {
                    PushRect(UI.Group, Element->Rect, Element->Color);
                }
    
                if (Element->Flags & RENDER_TEXT_UI_FLAG) {
                    game_font* Font = GetAsset(UI.Group->Assets, Element->Font);
                    float OffsetHeight = GetCharMaxHeight(Font, Element->Points);
                    PushText(UI.Group, Position + V2(0, OffsetHeight), Element->Name, 
                        .Color = Element->Color, .Font = Element->Font, .Points = Element->Points);
                }
            }
        }

        Element = Element->Next;
    }
}

const float SIDEBAR_WIDTH = 10.0f;
void UISidebar(ui_axis Axis) {
    Assert(UI.Tree.Parent != NULL);
    ui_axis OppositeAxis = Opposite(Axis);

    ui_alignment Alignments[2];
    Alignments[Axis] = ui_alignment_free;
    Alignments[OppositeAxis] = ui_alignment_max;

    float ParentsSize[2]       = { UI.Tree.Parent->Rect.Width, UI.Tree.Parent->Rect.Height };
    float ParentsParentSize[2] = { (float)UI.Group->Width, (float)UI.Group->Height };
    if (UI.Tree.Parent->Parent != NULL) {
        ParentsSize[0] = UI.Tree.Parent->Parent->Rect.Width;
        ParentsSize[1] = UI.Tree.Parent->Parent->Rect.Height;
    }
    
    float SideBarSize = ParentsParentSize[Axis] / ParentsSize[Axis] * ParentsParentSize[Axis];
    ui_size Size[2] = {};
    Size[Axis] = UISizeNull(SideBarSize);
    Size[OppositeAxis] = UISizePixels(SIDEBAR_WIDTH);
    ui_element* Element = PushUIElement(
        "Sidebar",
        .AlignmentX = Alignments[axis_x],
        .AlignmentY = Alignments[axis_y],
        .Flags = RENDER_RECT_UI_FLAG,
        .SizeX = Size[axis_x],
        .SizeY = Size[axis_y]
    );

    if (Element->Parent->Hovered && UI.Input->Mouse.Wheel != 0) {
        float Increment = (UI.Input->Mouse.Wheel > 0 ? -20.0f : 20.0f) / (ParentsParentSize[Axis] - SideBarSize);
        Element->Scroll = Clamp(Element->Scroll + Increment, 0.0f, 1.0f);
    }

    float Alpha = Element->Hovered ? 0.75f : 0.5f;
    if (Element->Dragged) {
        float Delta;
        if (Axis == axis_x) Delta = UI.Input->Mouse.Cursor.X - UI.Input->Mouse.LastCursor.X;
        else                Delta = UI.Input->Mouse.Cursor.Y - UI.Input->Mouse.LastCursor.Y;
        Element->Scroll = Clamp(Element->Scroll + Delta / (ParentsParentSize[Axis] - SideBarSize), 0.0f, 1.0f);
        Alpha = 0.75f;
    }

    Element->Color = ChangeAlpha(White, Alpha);

    Element->Parent->RelativePosition[Axis] = Element->Scroll * (ParentsParentSize[Axis] - ParentsSize[Axis]);
    Element->RelativePosition[Axis] = -Element->Parent->RelativePosition[Axis] + Element->Scroll * (ParentsParentSize[Axis] - SideBarSize);
}

struct ui_menu {
    ui_element* Element;
    ui_alignment Alignment;
    ui_axis StackAxis;

    ui_menu(
        const char* Text,
        ui_element_options Options = {}
    ) {
        Options.Color = ChangeAlpha(Black, 0.7f);
        StackAxis = Options.Stack;
        Alignment = StackAxis == axis_x ? Options.AlignmentX : Options.AlignmentY;
        ui_axis NoStack = Opposite(StackAxis);
        Options.Flags |= RENDER_RECT_UI_FLAG;
        ui_size Sizes[2] = {};
        if (StackAxis == axis_x) {
            Options.Flags |= STACK_CHILDREN_X_UI_FLAG;
            Sizes[axis_x] = UISizeSumChildren();
            Sizes[axis_y] = UISizeMaxChildren();
        }
        else {
            Options.Flags |= STACK_CHILDREN_Y_UI_FLAG;
            Sizes[axis_x] = UISizeMaxChildren();
            Sizes[axis_y] = UISizeSumChildren();
        }
        Options.SizeX = Sizes[axis_x];
        Options.SizeY = Sizes[axis_y];
        Element = PushUIElement(Text, Options);
        PushParent(Element);
    }

    ~ui_menu() {
        float ParentSize = StackAxis == axis_x ? UI.Group->Width : UI.Group->Height;
        if (Element->Parent != NULL) {
            ParentSize = axis_x ? Element->Parent->Rect.Width : Element->Parent->Rect.Height;
        }
        float ElementSize = StackAxis == axis_x ? Element->Rect.Width : Element->Rect.Height;
        if (ElementSize > ParentSize) {
            UISidebar(StackAxis);
            Element->Alignment[StackAxis] = ui_alignment_free;
        }
        else {
            Element->Alignment[StackAxis] = Alignment;
            Element->Scroll = 0;
            Element->RelativePosition[0] = 0;
            Element->RelativePosition[1] = 0;
        }
        PopParent();
    }
};

#define UIMenu(Name, ...) ui_menu(Name, { __VA_ARGS__ })

struct ui_dropdown {
    bool Expanded;

    ui_dropdown(const char* Text) {
        game_font* Font = GetAsset(UI.Group->Assets, Font_Menlo_Regular_ID);
        float Points = 12;
        float Width = 0, Height = 0;
        GetTextWidthAndHeight(Text, Font, Points, &Width, &Height);
        ui_size Sizes[2] = {
            UISizeMaxChildren(Width),
            UISizeSumChildren(Height)
        };
        
        ui_element* Element = PushUIElement(
            Text, 
            .AlignmentX = ui_alignment_min, .AlignmentY = ui_alignment_min,
            .Flags = RENDER_TEXT_UI_FLAG | STACK_CHILDREN_Y_UI_FLAG,
            .Points = Points,
            .SizeX = Sizes[axis_x], .SizeY = Sizes[axis_y]
        );
        Element->Rect.Width = Width;
        Element->Rect.Height = Height;

        bool Hovered = IsIn(Element->Rect, UI.Input->Mouse.Cursor);
        if (Hovered) {
            Element->Color = Yellow;
        }

        if (Hovered && UI.Input->Mouse.LeftClick.JustPressed) {
            Element->Expanded = !Element->Expanded;
        }

        Expanded = Element->Expanded;

        PushParent(Element);
    }

    ~ui_dropdown() {
        PopParent();
    }

    operator bool() const { return Expanded; }
};

#define UIDropdown(Name) ui_dropdown _##Name(#Name); _##Name

void _UIText(
    const char* Text,
    ui_element_options Options = {}
) {
    ui_size Sizes[2];
    UISizeText(Text, Options.Points, Sizes);
    Options.Flags |= RENDER_TEXT_UI_FLAG;
    Options.SizeX = Sizes[axis_x];
    Options.SizeY = Sizes[axis_y];
    ui_element* Element = PushUIElement(Text, Options);
}

#define UIText(Text, ...) _UIText(Text, { __VA_ARGS__ })

bool UIButton(const char* Text, float Points = 20.0f) {
    ui_size Sizes[2];
    UISizeText(Text, Points, Sizes);
    ui_element* Element = PushUIElement(
        Text, 
        .AlignmentX = ui_alignment_center, .AlignmentY = ui_alignment_center,
        .Flags = RENDER_TEXT_UI_FLAG,
        .Points = Points,
        .SizeX = Sizes[axis_x], .SizeY = Sizes[axis_y]
    );
    if (Element->Parent != NULL) {
        if (Element->Parent->Flags & STACK_CHILDREN_X_UI_FLAG) {
            Element->Alignment[0] = ui_alignment_free;
        }
        if (Element->Parent->Flags & STACK_CHILDREN_Y_UI_FLAG) {
            Element->Alignment[1] = ui_alignment_free;
        }
    }

    bool Hovered = IsIn(Element->Rect, UI.Input->Mouse.Cursor);
    if (Hovered) {
        Element->Color = Yellow;
    }

    return Element->Clicked;
}

void UIDebugValue(debug_entry* Entry) {
    ui_size Sizes[2] = {
        {ui_size_pixels, 0},
        {ui_size_pixels, 0},
    };
    UpdateAndSizeDebugEntry(UI.Group->DebugFont, Entry, &Sizes[axis_x].Value, &Sizes[axis_y].Value);

    ui_element* Element = PushUIElement(
        Entry->Name, 
        .AlignmentX = ui_alignment_min,
        .SizeX = Sizes[axis_x], .SizeY = Sizes[axis_y]
    );
    Element->DebugEntry = Entry;
    if (IsStructType(Entry->Type)) {
        Element->Size[axis_x] = UISizeMaxChildren(Sizes[axis_x].Value);
        Element->Size[axis_y] = UISizeSumChildren(Sizes[axis_y].Value);
        Element->Flags = STACK_CHILDREN_Y_UI_FLAG;
        if (Element->Hovered) Element->Color = Yellow;
        if (Entry->Value == NULL) {
            Element->Expanded = false;
        }
        else if (Element->Clicked) {
            Element->Expanded = !Element->Expanded;
        }
        if (Element->Expanded) {
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
                            debug_entry* ChildEntries = _AddDebugArray(
                                UI.DebugInfo, 
                                Member.Name, 
                                Member.MemberType, 
                                Member.Size, 
                                Pointer, 
                                Member.ArraySize,
                                Entry
                            );
                            for (int j = 0; j < Member.ArraySize; j++) {
                                UIDebugValue(&ChildEntries[j]);
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
    game_state* State = Memory->GameState;
    board* Board = &State->Board;
    float Time = State->Time;
    debug_info* DebugInfo = &Memory->DebugInfo;
    camera* Camera = &State->ActiveCamera;

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
        ui_menu MainMenu = UIMenu(
            "Main menu", 
            .AlignmentX = ui_alignment_center, .AlignmentY = ui_alignment_center,
            .MarginX = 50.0f, .MarginY = 20.0f
        );

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
            State->Exit = true;
        }
    }

    {
        ui_menu PlayMenu = UIMenu(
            "Play menu", 
            .AlignmentX = ui_alignment_center, .AlignmentY = ui_alignment_max, 
            .MarginX = 60.0f,
            .Stack = axis_x,
        );
        
        if (UIButton("Play")) {
            Board->Simulating = true;
        }

        if (UIButton("Stop")) {
            Board->Simulating = false;
        }

        if (UIButton("Clear")) {
            Board->Clearing = true;
        }

        if (!PlayMenu.Element->Hovered && !Board->Simulating) {
            float Left = 0.5f*Group->Width - 0.5f*Board->Width*CELL_SIZE;
            float Top = 0.5f*Group->Height - 0.5f*Board->Height*CELL_SIZE;

            int32 MouseRow = (Input->Mouse.Cursor.Y - Top) / CELL_SIZE;
            int32 MouseCol = (Input->Mouse.Cursor.X - Left) / CELL_SIZE;
            
            if (IsValidCell(Board, MouseRow, MouseCol)) {
                if (Input->Mouse.LeftClick.IsDown) {
                    SetCell(Board, MouseRow, MouseCol, true);
                }

                if (Input->Mouse.RightClick.IsDown) {
                    SetCell(Board, MouseRow, MouseCol, false);
                }
            }
        }

        DEBUG_VALUE(State->Board.Simulating, bool);
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
            double x = (State->dt - 1.8) / 1.1;
            DebugAlpha += 0.5 * exp(- x * x);
        }
        else DebugAlpha = 1.0;

        if (Input->Keyboard.N.JustPressed) Group->DebugNormals = !Group->DebugNormals;
        if (Input->Keyboard.B.JustPressed) Group->DebugBones = !Group->DebugBones;
        if (Input->Keyboard.C.JustPressed) Group->DebugColliders = !Group->DebugColliders;
        
        PushDebugGrid(Group, DebugAlpha);

        // Axes
        v2 XAxis = V2(cos(Camera->Angle * Degrees), sin(Camera->Angle * Degrees) * sin(Camera->Pitch * Degrees));
        v2 YAxis = V2(0.0, -cos(Camera->Pitch * Degrees));
        v2 ZAxis = V2(-sin(Camera->Angle * Degrees), sin(Camera->Pitch * Degrees) * cos(Camera->Angle * Degrees));
        v2 AxisOrigin = V2(Group->Width - 0.08 * (float)Group->Height - 10.0, 0.1 * (float)Group->Height);
        PushDebugVector(Group, 0.08 * Group->Height * XAxis, AxisOrigin, ChangeAlpha(Red, DebugAlpha));
        PushDebugVector(Group, 0.08 * Group->Height * YAxis, AxisOrigin, ChangeAlpha(Green, DebugAlpha));
        PushDebugVector(Group, 0.08 * Group->Height * ZAxis, AxisOrigin, ChangeAlpha(Blue, DebugAlpha));

        // Debug camera basis
        // PushDebugVector(Group, Camera->Basis, Camera->Basis.X, V3(0,0,0), World_Coordinates, Yellow);
        // PushDebugVector(Group, Camera->Basis, Camera->Basis.Y, V3(0,0,0), World_Coordinates, Magenta);
        // PushDebugVector(Group, Camera->Basis, Camera->Basis.Z, V3(0,0,0), World_Coordinates, Cyan);
        
        ui_menu DebugMenu = UIMenu(
            "Debug Menu",
            .AlignmentX = ui_alignment_min, .AlignmentY = ui_alignment_min,
            .MarginX = 5.0f
        );

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
            DEBUG_VALUE(VertexArena[vertex_layout_v2_id], memory_arena);
            DEBUG_VALUE(VertexArena[vertex_layout_v2_v2_id], memory_arena);
            DEBUG_VALUE(VertexArena[vertex_layout_v3_id], memory_arena);
            DEBUG_VALUE(VertexArena[vertex_layout_v3_v2_id], memory_arena);
            DEBUG_VALUE(VertexArena[vertex_layout_v3_v2_v3_id], memory_arena);
            DEBUG_VALUE(VertexArena[vertex_layout_bones_id], memory_arena);
            nEntries = DebugInfo->nEntries;
            for (; i < nEntries; i++) {
                debug_entry* Entry = &DebugInfo->Entries[i];
                UIDebugValue(Entry);
            }
        }
        
        // Debug Framebuffer
        PushDebugTarget(Group, Target_Postprocessing_Outline);
    }

    if (UI.Tree.Current) UI.Tree.Current->Next = 0;
    UI.Tree.Last = UI.Tree.Current;

    ComputeSizes();
    ComputeLayout();
    RenderUI();

    UI.Tree.Current = NULL;
    UI.Tree.Parent = NULL;

    Assert(UI.IDStack.n == 0);
}

#endif