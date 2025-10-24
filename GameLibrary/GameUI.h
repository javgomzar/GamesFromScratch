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
    Result->ID = ID;
    Result->Index = UI.CurrentIndex;
    Result->Scroll = 0;
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
    Element->Hovered = IsInside(Element->Rect, UI.Input->Mouse.Cursor);
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
    game_font_id FontID = Font_Menlo_Regular_ID;
    game_font* Font = GetAsset(UI.Group->Assets, FontID);

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
        Size[axis_x],
        Size[axis_y],
        Alignments[0],
        Alignments[1],
        RENDER_RECT_UI_FLAG
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

struct UIMenu {
    ui_element* Element;
    ui_alignment Alignment;
    ui_axis StackAxis;

    UIMenu(
        const char* Text,
        ui_axis Stack = axis_y, 
        ui_alignment AlignmentX = ui_alignment_center,
        ui_alignment AlignmentY = ui_alignment_center,
        float MarginX = 10.0f,
        float MarginY = 10.0f,
        color C = ChangeAlpha(Black, 0.7f)
    ) {
        StackAxis = Stack;
        Alignment = Stack == axis_x ? AlignmentX : AlignmentY;
        ui_axis NoStack = Opposite(Stack);
        ui_flags Flags = RENDER_RECT_UI_FLAG;
        ui_size Sizes[2] = {};
        if (Stack == axis_x) {
            Flags |= STACK_CHILDREN_X_UI_FLAG;
            Sizes[axis_x] = UISizeSumChildren();
            Sizes[axis_y] = UISizeMaxChildren();
        }
        else {
            Flags |= STACK_CHILDREN_Y_UI_FLAG;
            Sizes[axis_x] = UISizeMaxChildren();
            Sizes[axis_y] = UISizeSumChildren();
        }
        Element = PushUIElement(Text, Sizes[0], Sizes[1], AlignmentX, AlignmentY, Flags);
        Element->Margins[axis_x] = MarginX;
        Element->Margins[axis_y] = MarginY;
        Element->Color = C;
        PushParent(Element);
    }

    ~UIMenu() {
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

        bool Hovered = IsInside(Element->Rect, UI.Input->Mouse.Cursor);
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
        ui_alignment_center, 
        ui_alignment_center,
        RENDER_TEXT_UI_FLAG
    );
    if (Element->Parent != NULL) {
        if (Element->Parent->Flags & STACK_CHILDREN_X_UI_FLAG) {
            Element->Alignment[0] = ui_alignment_free;
        }
        if (Element->Parent->Flags & STACK_CHILDREN_Y_UI_FLAG) {
            Element->Alignment[1] = ui_alignment_free;
        }
    }
    Element->Points = Points;

    bool Hovered = IsInside(Element->Rect, UI.Input->Mouse.Cursor);
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

    ui_element* Element = PushUIElement(Entry->Name, Sizes[0], Sizes[1], ui_alignment_min, ui_alignment_free);
    Element->DebugEntry = Entry;
    if (IsStructType(Entry->Type)) {
        Element->Size[axis_x] = UISizeMaxChildren(Sizes[axis_x].Value);
        Element->Size[axis_y] = UISizeSumChildren(Sizes[axis_y].Value);
        Element->Flags = STACK_CHILDREN_Y_UI_FLAG;
        if (Element->Hovered) Element->Color = Yellow;
        if (Entry->Value == NULL) {
            Element->Expand = false;
        }
        else if (Element->Clicked) {
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

enum settings_section {
    Graphics_Settings,
    Audio_Settings,
    Game_Settings,
};

void SettingsUI() {
    static settings_section ActiveSection = Graphics_Settings;
    {
        UIMenu SectionTabs = UIMenu("Section tabs", axis_x, ui_alignment_center, ui_alignment_min);
        
        if (UIButton("Graphics")) {
            ActiveSection = Graphics_Settings;
        }
        
        if (UIButton("Audio")) {
            ActiveSection = Audio_Settings;
        }

        if (UIButton("Game")) {
            ActiveSection = Game_Settings;
        }
    }

    UIMenu SettingsMenu = UIMenu("Settings menu", axis_y, ui_alignment_center, ui_alignment_center);
    
    switch(ActiveSection) {
        case Graphics_Settings: {
            UIText("GRAPHICS");
        } break;

        case Audio_Settings: {
            UIText("AUDIO");
        } break;

        case Game_Settings: {
            UIText("GAME");
        } break;

        default: Raise("Invalid settings section.");
    }
}

void UpdateMainMenuUI(
    game_memory* Memory,
    game_input* Input
) {
    game_state* State = Memory->GameState;

    UIText("Untitled game", ui_alignment_center, ui_alignment_center, White, 100);
    static bool Settings = false, ClassSelection = false;
    
    {
        UIMenu StartMenu = UIMenu("Start menu", axis_x, ui_alignment_center, ui_alignment_max, 50.0f, 20.0f);

        if (UIButton("New game")) {
            ClassSelection = true;
        }
        
        if (UIButton("Continue")) {
            
        }
        
        if (UIButton("Settings")) {
            Settings = !Settings;
        }
        
        if (UIButton("Exit")) {
            State->Exit = true;
        }
    }

    if (Settings) {
        SettingsUI();
    }

    if (ClassSelection) {
        UIMenu ClassSelectionMenu = UIMenu("Class selection menu", axis_y, ui_alignment_center, ui_alignment_center, 50.0f, 20.0f);

        for (int i = 0; i < character_class_count; i++) {
            if (UIButton(ClassNames[i])) {
                character_class Class = (character_class)i;
                character* Character = AddCharacter(&State->EntityManager, Class, V3(0,0,0));
                ClassSelection = false;

                room_type FirstRoomType = Room_Type_Combat;
                switch(Class) {
                    case Class_Wizard: {
                        FirstRoomType = Room_Type_Wizard;
                    } break;

                    case Class_Rogue: {
                        FirstRoomType = Room_Type_Merchant;
                        State->Gold = 10;
                    } break;
                }
                
                RandomizeLevel(&State->Level, FirstRoomType);
                State->CurrentRoom = &State->Level.Rooms[0];
                Transition(State, GetStateType(FirstRoomType));
            }
        }
    }
}

void UpdateCombatUI(
    game_memory* Memory,
    game_input* Input
) {
    render_group* Group = &Memory->RenderGroup;
    game_state* State = (game_state*)Memory->Permanent.Base;
    game_entity_manager* EntityManager = &State->EntityManager;
    float Time = State->Time;
    game_combat* Combat = &State->Combat;
    debug_info* DebugInfo = &Memory->DebugInfo;
    game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);

    // Combat menu
    if (State->Combat.Active) {
        combatant* ActiveCombatant = Combat->Turn.Attacker;
        float CombatMenuWidth = 0;
        if (ActiveCombatant->Type == Combatant_Type_Player) {
            UIMenu CombatMenu = UIMenu("Combat menu", axis_y, ui_alignment_min, ui_alignment_max, 80.0f, 20.0f);
            CombatMenuWidth = CombatMenu.Element->Rect.Width;

            if (UIButton("Attack")) {
                State->Combat.Turn.Action = combatant_action_attack;
            }
            for (int i = 0; i < MAX_COMBATANT_SPELLS; i++) {
                if (ActiveCombatant->Spells[i] != Spell_Empty) {
                    if (UIButton("Magic")) {
                        State->Combat.Turn.Action = combatant_action_magic;
                    };
                    break;
                }
            }

            for (int i = 0; i < 3; i++) {
                if (State->Inventory[i] != Item_Type_None) {
                    if (UIButton("Items")) {
                        State->Combat.Turn.Action = combatant_action_items;
                    }
                    break;
                }
            }

            if (UIButton("Flee")) {
                State->Combat.Turn.Action = combatant_action_flee;
            }

            if (Input->Mouse.RightClick.JustPressed && State->Combat.Turn.Action != combatant_action_empty) {
                State->Combat.Turn.Action = combatant_action_empty;
                State->Combat.Turn.Spell = Spell_Empty;
            }

            if (State->Combat.Turn.Action != combatant_action_empty) {
                const char* Strings[] = {
                    "Attack", "Magic", "Items", "Flee"
                };
                float Width = 0, Height = 0;
                GetTextWidthAndHeight(Strings[State->Combat.Turn.Action - 1], Font, 20.0f, &Width, &Height);
                float PosY = Group->Height - CombatMenu.Element->Rect.Height + (float)State->Combat.Turn.Action * (Height + 20.0f);
                triangle2 Triangle = {
                    V2(20, PosY - 10),
                    V2(20, PosY + 10),
                    V2(50, PosY),
                };
                PushLine(
                    Group, 
                    V2(0, PosY), 
                    V2((CombatMenu.Element->Rect.Width + Width) / 2.0f, PosY), 
                    White, 
                    2.0f, 
                    SORT_ORDER_DEBUG_OVERLAY + 1.0f
                );
            }
        }

        // Magic menu
        if (State->Combat.Turn.Action == combatant_action_magic) {
            UIMenu MagicMenu = UIMenu("Magic Menu", axis_y, ui_alignment_free, ui_alignment_max, 80.0f, 20.0f);

            MagicMenu.Element->RelativePosition[axis_x] = CombatMenuWidth;

            for (int i = 0; i < MAX_COMBATANT_SPELLS; i++) {
                spell_id SpellID = ActiveCombatant->Spells[i];
                if (SpellID != Spell_Empty) {
                    if (UIButton(Spells[SpellID].Name)) {
                        State->Combat.Turn.Spell = SpellID;
                    }
                }
            }

            if (State->Combat.Turn.Spell != Spell_Empty) {
                spell Spell = Spells[State->Combat.Turn.Spell];
                float Width = 0, Height = 0;
                GetTextWidthAndHeight(Spell.Name, Font, 20.0f, &Width, &Height);
                float PosY = Group->Height - MagicMenu.Element->Rect.Height + (float)Spell.ID * (Height + 20.0f);
                triangle2 Triangle = {
                    V2(20, PosY - 10),
                    V2(20, PosY + 10),
                    V2(50, PosY),
                };
                PushLine(
                    Group, 
                    V2(0, PosY), 
                    V2((MagicMenu.Element->Rect.Width + Width) / 2.0f, PosY), 
                    White, 
                    2.0f, 
                    SORT_ORDER_DEBUG_OVERLAY + 1.0f
                );
            }
        }
        
        // Next turns menu
        {
            UIMenu NextTurnsMenu = UIMenu("Next turns menu", axis_y, ui_alignment_max, ui_alignment_center, 20.0f, 20.0f);
    
            const int TurnsShown = 8;
            char TurnBuffer[64];
            v2 Position = V2(UI.Group->Width - 300.0f, 250.0f);
            for (int i = 0; i < TurnsShown; i++) {
                turn Turn = Combat->NextTurns[i];
                sprintf_s(TurnBuffer, "Turn %d: %s", Turn.Index, Turn.Attacker->Entity->Name);
                PushText(UI.Group, Position, Font_Menlo_Regular_ID, TurnBuffer, White, 10.0f);
                Position.Y += 20.0f;
            }
        }

        // YOU DIED
        bool Alive = false;
        for (int i = 0; i < Combat->Combatants.Count; i++) {
            combatant* Combatant = &Combat->Combatants.Content[i];
            if (Combatant->Type == Combatant_Type_Player && !Combatant->AlteredState[altered_state_dead]) {
                Alive = true;
                break;
            }
        }

        if (!Alive) {
            UIText("YOU DIED", ui_alignment_center, ui_alignment_center, Red, 120);
            if (UIButton("Return to main menu")) {
                EndCombat(Combat, &State->EntityManager, &State->Gold, false);
                uint32 nWeapons = State->EntityManager.Weapons.Count;
                uint32 Index = 0;
                while (nWeapons > 0) {
                    weapon* Weapon = &State->EntityManager.Weapons.List[Index++];
                    if (Weapon->Entity != NULL) {
                        nWeapons--;
                    }
                    else continue;

                    RemoveEntity(&State->EntityManager, Weapon->Entity->ID);
                }
                uint32 nEnemies = State->EntityManager.Enemies.Count;
                Index = 0;
                while (nEnemies > 0) {
                    enemy* Enemy = &State->EntityManager.Enemies.List[Index++];
                    if (Enemy->Entity != NULL) {
                        nEnemies--;
                    }
                    else continue;

                    RemoveEntity(&State->EntityManager, Enemy->Entity->ID);
                }
                Transition(State, Game_State_Main_Menu);
            }
        }
    }
}

void UpdateMapUI(
    game_memory* Memory,
    game_input* Input
) {
    render_group* Group = &Memory->RenderGroup;
    game_state* State = Memory->GameState;
    level* Level = &State->Level;
    room* CurrentRoom = State->CurrentRoom;

    room* FirstRoom = &Level->Rooms[0];
    room* LastRoom = &Level->Rooms[1];

    v2 Center = V2(0.5f * (float)Group->Width, 0.5f * (float)Group->Height);
    v2 Position = Center + V2(-50, (Level->Depth + 1.5f)*50.0f);

    uint32 Index = 0;
    for (int i = 0; i <= Level->Depth; i++) {
        int32 nRow = Level->nInRow[i];
        int32 nNextRow = i == Level->Depth ? 1 : Level->nInRow[i+1];
        Position.X = Center.X - 50.0f - (nRow - 1) * 50.0f;
        for (int j = 0; j < nRow; j++) {
            room* Room = &Level->Rooms[Index++];
            PushRoom(Group, Room, Position);
            rectangle Rect = { Position.X, Position.Y, 99, 99 };
            if (Room == CurrentRoom) {
                PushRectOutline(Group, Rect, White);
            }
            for (int k = 0; k < Room->nPrevious; k++) {
                if (Room->Previous[k] == CurrentRoom && IsInside(Rect, Input->Mouse.Cursor)) {
                    PushRectOutline(Group, Rect, Yellow);
                    if (Input->Mouse.LeftClick.JustPressed) {
                        State->CurrentRoom = Room;
                        game_state_type NextState = GetStateType(Room->Type);
                        Transition(State, NextState);
                    }
                    break;
                }
            }
            for (int k = 0; k < Room->nNext; k++) {
                room* Next = Room->Next[k];
                v2 V = V2((Next->RowIndex - Room->RowIndex)*100.0f + 50.0f*(nRow - nNextRow), -50);
                PushDebugVector(Group, V, Position + V2(50, 0), Black);
            }
            Position.X += 100.0f;
        }
        Position.Y -= 150.0f;
    }
}

void UpdateTradeUI(
    game_memory* Memory,
    game_input* Input
) {
    render_group* Group = &Memory->RenderGroup;
    game_state* State = Memory->GameState;
    camera* Camera = State->ActiveCamera;

    switch(State->CurrentRoom->Type) {
        case Room_Type_Merchant: {
            UIMenu StoreMenu = UIMenu("Store menu", axis_y, ui_alignment_center, ui_alignment_center, 10.0f, 20.0f);

            char Buffer[16];
            for (int i = 0; i < 3; i++) {
                item Item = Items[State->Store[i]];
                PushBitmap(Group, Item.BitmapID, {400.0f, 220.0f + 60.0f*i, 60.0f, 60.0f}, SORT_ORDER_DEBUG_OVERLAY + 20.0f);
                if (UIButton(Item.Name)) {
                    for (int j = 0; j < 3; j++) {
                        item_type InventoryItem = State->Inventory[j];
                        if (InventoryItem == Item_Type_None) {
                            State->Inventory[j] = Item.Type;
                            State->Gold -= 10;
                            Transition(State, Game_State_Map);
                            break;
                        }
                    }
                }
            }

            if (UIButton("Skip")) {
                Transition(State, Game_State_Map);
            }
        } break;

        case Room_Type_Wizard: {
            UIMenu WizardMenu = UIMenu("Wizard menu", axis_y);

            spell_id SelectedSpell = Spell_Empty;

            if (UIDropdown(Fire)) {
                for (int i = 0; i < nFireSpells; i++) {
                    spell Spell = Spells[FireSpellIDs[i]];
                    if (UIButton(Spell.Name)) {
                        SelectedSpell = Spell.ID;
                    }
                }
            }

            if (UIDropdown(Earth)) {
                for (int i = 0; i < nEarthSpells; i++) {
                    spell Spell = Spells[EarthSpellIDs[i]];
                    if (UIButton(Spell.Name)) {
                        SelectedSpell = Spell.ID;
                    }
                }
            }

            if (UIDropdown(Air)) {
                for (int i = 0; i < nAirSpells; i++) {
                    spell Spell = Spells[AirSpellIDs[i]];
                    if (UIButton(Spell.Name)) {
                        SelectedSpell = Spell.ID;
                    }
                }
            }

            if (UIDropdown(Water)) {
                for (int i = 0; i < nWaterIceSpells; i++) {
                    spell Spell = Spells[WaterIceSpellIDs[i]];
                    if (UIButton(Spell.Name)) {
                        SelectedSpell = Spell.ID;
                    }
                }
            }

            if (UIDropdown(Aether)) {
                for (int i = 0; i < nLifeDeathSpells; i++) {
                    spell Spell = Spells[LifeDeathSpellIDs[i]];
                    if (UIButton(Spell.Name)) {
                        SelectedSpell = Spell.ID;
                    }
                }
            }

            if (UIDropdown(Time)) {
                for (int i = 0; i < nTimeSpells; i++) {
                    spell Spell = Spells[TimeSpellIDs[i]];
                    if (UIButton(Spell.Name)) {
                        SelectedSpell = Spell.ID;
                    }
                }
            }

            if (SelectedSpell != Spell_Empty) {
                character* Character = &State->EntityManager.Characters.List[0];
                for (int i = 0; i < MAX_COMBATANT_SPELLS; i++) {
                    if (Character->Spells[i] == Spell_Empty) {
                        Character->Spells[i] = SelectedSpell;
                        break;
                    }
                }

                Transition(State, Game_State_Map);
            }
            
            if (UIButton("Skip")) {
                Transition(State, Game_State_Map);
            }
            
            transform T = Transform(Camera->Position - Camera->Basis.Z);
            T.Scale = Scale(5.0f, 5.0f, 5.0f);
            T.Rotation = Quaternion(Camera->Angle * Degrees, V3(0, 1, 0));
        } break;

        case Room_Type_Blacksmith: {
            UIMenu BlacksmithMenu = UIMenu("Blacksmith menu", axis_x);

            static bool Improve = false;
            if (UIButton("Improve")) {
                Improve = true;
            }
            static bool Trade = false;
            if (UIButton("Trade")) {
                Trade = true;
            }

            if (UIButton("Skip")) {
                Transition(State, Game_State_Map);
            }
        } break;

        case Room_Type_Quest: {
            UIMenu QuestMenu = UIMenu("Quest menu", axis_x);

            static bool Accept = false;
            if (UIButton("Accept")) {
                Accept = true;
            }
            static bool Decline = false;
            if (UIButton("Decline")) {
                Decline = true;
            }

            if (UIButton("Skip")) {
                Transition(State, Game_State_Map);
            }
        } break;
    }
}

void UpdateUI(
    game_memory* Memory,
    game_input* Input
) {
    render_group* Group = &Memory->RenderGroup;
    game_state* State = Memory->GameState;
    game_entity_manager* EntityManager = &State->EntityManager;
    float Time = State->Time;
    game_combat* Combat = &State->Combat;
    debug_info* DebugInfo = &Memory->DebugInfo;
    camera* Camera = State->ActiveCamera;

    BeginContext(Memory, Input);

    UI.Tree.First = 0;

    switch(State->Type) {
        case Game_State_Main_Menu: {
            UpdateMainMenuUI(Memory, Input);
        } break;

        case Game_State_Map: {
            UpdateMapUI(Memory, Input);
        } break;

        case Game_State_Combat: {
            UpdateCombatUI(Memory, Input);
        } break;

        case Game_State_Camp: {
            // TODO: do something here instead of waiting
            if (State->CampTime > 2.0f) {
                Transition(State, Game_State_Map);
            }
            else {
                UIText("Resting...", ui_alignment_center, ui_alignment_center, Black);
                State->CampTime += State->dt;
            }
        } break;

        case Game_State_Trade: {
            UpdateTradeUI(Memory, Input);
        } break;

        case Game_State_Credits: {
            UIText("JGZ");
        } break;
    }

    // In-game menu
    if (State->Type != Game_State_Main_Menu && State->Type != Game_State_Credits) {
        static bool ShowMenu = false;
        bool MenuInput = Input->Mode == Keyboard && Input->Keyboard.Escape.JustPressed ||
                        Input->Mode == Controller && Input->Controller.Start.JustPressed;
        if (MenuInput) {
            ShowMenu = !ShowMenu;
        }
        
        static bool Settings = false;
        if (ShowMenu) {
            UIMenu MainMenu = UIMenu("Main menu", axis_y, ui_alignment_center, ui_alignment_center, 50.0f, 20.0f);

            if (UIButton("Save game")) {
                // TODO: Save game
            }

            if (UIButton("Load game")) {
                // TODO: Load game
            }

            if (UIButton("Settings")) {
                Settings = !Settings;
            }
            
            if (UIButton("Main menu")) {
                Transition(State, Game_State_Main_Menu);
                ShowMenu = false;
            }

            if (UIButton("Exit")) {
                State->Exit = true;
            }
        }

        if (Settings) {
            SettingsUI();

            if (Input->Keyboard.Escape.JustPressed) {
                Settings = false;
            }
        }

        // Gold
        char Buffer[64];
        sprintf_s(Buffer, "Gold: %d", State->Gold);
        PushText(Group, V2(Group->Width - 200, 50), Font_Menlo_Regular_ID, Buffer, Yellow);

        // Inventory
        static bool Selected[3] = {};
        for (int i = 0; i < 3; i++) {
            item Item = Items[State->Inventory[i]];
            if (Item.Type != Item_Type_None) {
                rectangle Rect = { 60.0f*i, 0.0f, 60.0f, 60.0f };
                PushBitmap(Group, Item.BitmapID, Rect);
                if (State->Type == Game_State_Combat && State->Combat.Turn.Action == combatant_action_items) {
                    bool Hovered = IsInside(Rect, Input->Mouse.Cursor);
                    if (Hovered || Selected[i]) {
                        PushRectOutline(Group, Rect, Yellow);
                    }

                    if (Hovered && Input->Mouse.LeftClick.JustPressed) {
                        State->Combat.Turn.UsedItem = &State->Inventory[i];
                        Selected[i] = true;
                        Selected[(i + 1) % 3] = false;
                        Selected[(i + 2) % 3] = false;
                    }
                }
                else {
                    Selected[i] = false;
                }
            }
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

            DEBUG_VALUE(Combat->TurnsArena, memory_arena);

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
            character* Characters[MAX_ENEMIES] = {};
            uint32 nCharacters = 0;
            uint32 Index = 0;
            while (nCharacters < EntityManager->Characters.Count && Index < MAX_ENTITIES) {
                character* Character = &EntityManager->Characters.List[Index++];
                if (Character->Entity && Character->Entity->Active) {
                    Characters[nCharacters++] = Character;
                }
            }
            DEBUG_POINTER_ARRAY(Characters, EntityManager->Characters.Count, character);

            if (EntityManager->Enemies.Count > 0) {
                enemy* Enemies[MAX_ENEMIES] = {};
                uint32 nEnemies = 0;
                uint32 Index = 0;
                while (nEnemies < EntityManager->Enemies.Count && Index < MAX_ENTITIES) {
                    enemy* Enemy = &EntityManager->Enemies.List[Index++];
                    if (Enemy->Entity && Enemy->Entity->Active) {
                        Enemies[nEnemies++] = Enemy;
                    }
                }
                DEBUG_POINTER_ARRAY(Enemies, EntityManager->Enemies.Count, enemy);
            }
            
            nEntries = DebugInfo->nEntries;
            for (; i < nEntries; i++) {
                debug_entry* Entry = &DebugInfo->Entries[i];
                UIDebugValue(Entry);
            }
        }

        if (Combat->Active) {
            if (UIDropdown(Combat)) {
                DEBUG_ARRAY(Combat->Combatants.Content, Combat->Combatants.Count, combatant);

                nEntries = DebugInfo->nEntries;
                for (; i < nEntries; i++) {
                    debug_entry* Entry = &DebugInfo->Entries[i];
                    UIDebugValue(Entry);
                }
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

    UI.Tree.Current = NULL;
    UI.Tree.Parent = NULL;

    Assert(UI.IDStack.n == 0);
}

#endif