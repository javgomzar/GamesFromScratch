#ifndef GAME_UI
#define GAME_UI

#include "GameLibrary.h"

enum UI_Alignment {
    UI_Alignment_Free,
    UI_Alignment_Center,
    UI_Alignment_Min,
    UI_Alignment_Max
};

/*
    Returns the relative offset of a `contained` length when fitting it inside a `container` following `alignment` (Center, Min or Max).
    Optional `pad` input adds padding to this computation.
        - Example: `Align(UI_Alignment_Center, 100.0f, 50.0f) == 25.0f`
*/
float Align(UI_Alignment alignment, float container, float contained, float pad = 0.0f) {
    switch(alignment) {
        case UI_Alignment_Center: { return 0.5f * (container - contained); } break;
        case UI_Alignment_Min:    { return pad; } break;
        case UI_Alignment_Max:    { return container - contained - pad; } break;
        default: Raise("Invalid alignment.");
    }
    return 0;
}

enum UI_Size_Type {
    UI_Size_Null,
    UI_Size_Pixels,
    UI_Size_Text,
    UI_Size_Proportion,
    UI_Size_Sum,
    UI_Size_Max
};

struct UI_Size {
    UI_Size_Type type;
    float value;
    float min = 0;
    float max = FLT_MAX;
};

// This size will be skipped when computing children dependent sizes and layout
UI_Size UISizeNull(float value = 0.0f) {
    return { UI_Size_Null, value };
}

UI_Size UISizePixels(float value) {
    return { UI_Size_Pixels, value, value, value };
}

UI_Size UISizePercentParent(float value, float min = 0.0f, float max = FLT_MAX) {
    return { UI_Size_Proportion, value, min, max };
}

UI_Size UISizeMaxChildren(float min = 0.0f, float max = FLT_MAX) {
    return { UI_Size_Max, min, min, max };
}

UI_Size UISizeSumChildren(float initial_value = 0.0f, float max = FLT_MAX) {
    return { UI_Size_Sum, initial_value, initial_value, max };
}

FLAGS(UI_Flags,
    Drag
);

FLAGS(UI_Draw,
    Rect,
    Text,
    Fillbar,
    Slider,
    Checkbox
);

struct UI_Element {
    String name;
    UI_Flags flags;
    UI_Element* parent;
    UI_Element* previous;
    UI_Element* next;

    Color color;
    Font* font;
    Rect rect;
    float relative_position[2];
    float pad[2];
    float points;
    float scroll;

    UI_Size size[2];
    UI_Alignment alignment[2];
    uint64 last_frame;

    bool hovered;
    bool clicked;
    bool dragged;
    bool expanded;
};

const int MAX_UI_ELEMENTS = 256;
struct UI_Hierarchy {
    Hash_Table<String, UI_Element> elements;
    UI_Element* parent;
    UI_Element* first;
    UI_Element* current;
    UI_Element* last;
    uint32 count;
};

struct UI_Context {
    UI_Hierarchy tree;
    Render_Group* group;
    Input* input;
    Debug_Info* debug_info;
    uint64 frame;
};

static UI_Context ui_context = {};

void UISizeText(const char* text, int points, UI_Size& size_x, UI_Size& size_y) {
    Rect rect;
    GetTextWidthAndHeight(text, ui_context.group->debug_font, points, &rect.width, &rect.height);
    size_x.type = UI_Size_Text;
    size_x.value = rect.width;
    size_y.type = UI_Size_Text;
    size_y.value = rect.height;
}

struct UI_Element_Options {
    UI_Alignment alignment_x = UI_Alignment_Free;
    UI_Alignment alignment_y = UI_Alignment_Free;
    Color        color      = Color::WHITE;
    UI_Flags     flags      = UI_Flags::none;
    UI_Draw      draw_flags = UI_Draw::none;
    Font*        font       = nullptr;
    float        margin_x   = 0.0f;
    float        margin_y   = 0.0f;
    float        points     = 20.0f;
};

UI_Element* AddUIElement(
    String name,
    UI_Size sizes[2],
    UI_Element_Options options,
    int children_count,
    ...
) {
    UI_Element* element = ui_context.tree.elements.Insert(name);
    element->last_frame = ui_context.frame;
    element->size[0] = sizes[0];
    element->size[1] = sizes[1];
    element->alignment[0] = options.alignment_x;
    element->alignment[1] = options.alignment_y;
}

UI_Element* _PushUIElement(
    const char* Name,
    UI_Element_Options Options = {}
) {
    bool Found = false;
    uint32 ID = GetID(Name);
    UI_Element* Element = 0;
    for (int i = 0; i < MAX_UI_ELEMENTS; i++) {
        Element = &UI.Tree.Elements[i];
        if (ID == Element->ID) {
            Found = true;
            break;
        }
    }
    if (!Found) Element = NewUIElement(ID);
    if (!UI.Tree.First) UI.Tree.First = Element;

    Element->Parent = UI.Tree.Parent;
    UI_Size Sizes[2] = { Options.SizeX, Options.SizeY };
    Element->Size[axis_x] = Sizes[axis_x];
    Element->Size[axis_y] = Sizes[axis_y];
    Element->Margins[axis_x] = Options.MarginX;
    Element->Margins[axis_y] = Options.MarginY;

    float* RectSizes[2] = { &Element->Rect.Width, &Element->Rect.Height };
    for (int i = 0; i < 2; i++) {
        switch(Sizes[i].Type) {
            case UI_Size_null:
            case UI_Size_percent_of_parent:
            case UI_Size_sum_of_children:
            case UI_Size_max_of_children: break;

            case UI_Size_text:
            case UI_Size_pixels: {
                Element->Size[i].value = Sizes[i].value;
                *RectSizes[i] = Sizes[i].value;
            } break;

            default: Raise("Invalid size type");
        }
    }

    strcpy(Element->Name, Name);
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

void BeginContext(Memory* memory, Input* input) {
    ui_context.group = &memory->render_group;
    ui_context.input = input;
    ui_context.debug_info = &memory->debug_info;
}

void ComputeSizes() {
    UI_Element* element = UI.Tree.First;
    if (!Element) return;

    float GroupSizes[2] = { (float)UI.Group->Width, (float)UI.Group->Height };

    // Go from top to bottom computing sizes that don't depend on children
    while (Element) {
        for (int i = 0; i < 2; i++) {
            switch(Element->Size[i].Type) {
                case UI_Size_null:
                case UI_Size_pixels:
                case UI_Size_text: {
                    if (i == axis_x) Element->Rect.Width = Element->Size[i].value;
                    else             Element->Rect.Height = Element->Size[i].value;
                } break;

                case UI_Size_percent_of_parent: {
                    UI_Element* Parent = Element->Parent;
                    float ParentSize = Parent ? Parent->Size[i].value : GroupSizes[i];
                    float ParentMargin = Parent ? Parent->Margins[i] : 0;

                    if (i == axis_x) Element->Rect.Width  = Element->Size[i].value * ParentSize - 2.0f * ParentMargin;
                    else             Element->Rect.Height = Element->Size[i].value * ParentSize - 2.0f * ParentMargin;
                } break;

                case UI_Size_max_of_children: 
                case UI_Size_sum_of_children: { 
                    Element->Size[i].value += Element->Margins[i];
                } break;
                
                default: Raise("Invalid size type.");
            }

            if (Element->Size[i].value > Element->Size[i].max) {
                Element->Size[i].value = Element->Size[i].max;
            }
            if (Element->Size[i].value < Element->Size[i].min) {
                Element->Size[i].value = Element->Size[i].min;
            }
        }

        if (!Element->Next) break;
        Element = Element->Next;
    }

    Assert(Element == UI.Tree.Last);

    // Go from bottom to top computing sizes that depend on children
    while (Element) {
        UI_Element* Parent = Element->Parent;
        if (Parent) {
            for (int i = 0; i < 2; i++) {
                if (Element->Size[i].Type != UI_Size_null) {
                    if (Parent->Size[i].Type == UI_Size_max_of_children) {
                        float Childvalue = Element->Size[i].value + 2.0f * Parent->Margins[i];
                        if (Childvalue > Parent->Size[i].value) Parent->Size[i].value = Childvalue;
                    }
                    else if (Parent->Size[i].Type == UI_Size_sum_of_children) {
                        Parent->Size[i].value += Element->Size[i].value + Parent->Margins[i];
                    }
                }
            }
            Parent->Rect.Width = Parent->Size[axis_x].value;
            Parent->Rect.Height = Parent->Size[axis_y].value;
        }
        
        if (!Element->Previous) break;
        Element = Element->Previous;
    }

    Assert(Element == UI.Tree.First);
    
    // Go back from top to bottom to compute parent dependent sizes
    while (Element) {
        for (int i = 0; i < 2; i++) {
            float Result = 0;
            if (Element->Size[i].Type == UI_Size_percent_of_parent) {
                UI_Element* Parent = Element->Parent;
                float ParentSize = Parent ? Parent->Size[i].value : GroupSizes[i];
                float ParentMargin = Parent ? Parent->Margins[i] : 0;
                Result = Element->Size[i].value * ParentSize - 2.0f * ParentMargin;

                if (i == axis_x) Element->Rect.Width  = Result;
                else             Element->Rect.Height = Result;
            }
        }
        
        if (!Element->Next) break;
        Element = Element->Next;
    }
}

void ComputeLayout() {
    // Compute relative positions
    UI_Element* Element = UI.Tree.First;
    if (!Element) return;

    while(Element) {
        // Some elements lay out their children
        if (Element->Flags & (ui_flag::stack_children_x | ui_flag::stack_children_y)) {
            ui_axis StackAxis = (Element->Flags & ui_flag::stack_children_x) ? axis_x : axis_y;

            float Nextvalue = Element->Margins[StackAxis] + Element->Size[StackAxis].min;
            UI_Element* Child = Element->Next;
            while (Child) {
                if (Child->Parent == Element) {
                    if (Child->Size[StackAxis].Type != UI_Size_null) {
                        Child->RelativePosition[StackAxis] = Nextvalue;
                        Nextvalue += Element->Margins[StackAxis] + Child->Size[StackAxis].value;
                    }
                }

                Child = Child->Next;
            };
        }

        // All elements have a relative position to their parent
        float ParentSizes[2]    = { (float)UI.Group->Width, (float)UI.Group->Height };
        float ParentMargins[2]  = { 0, 0 };
        float ParentPosition[2] = { 0, 0 };

        bool HasParent = Element->Parent;
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
            UI_Alignment Alignment = Element->Alignment[i];
            if (Alignment != UI_Alignment_free)
                ElementRectPos[i] += Align(Alignment, ParentSizes[i], ElementRectSize[i], ParentMargins[i]);
        }

        // Some children control their parents

        Element = Element->Next;
    }
}

void RenderUI() {
    rectangle Screen = {0, 0, (float)UI.Group->Width, (float)UI.Group->Height};

    UI_Element* Element = UI.Tree.First;
    while (Element) {
        v2 Position = LeftTop(Element->Rect);
        if (Intersect(Screen, Element->Rect)) {
            if (Element->DebugEntry) {
                PushDebugEntry(UI.Group, Element->DebugEntry, Position, Element->Color);
            }
            else {
                if (Element->Flags & ui_flag::render_rect) {
                    PushRect(UI.Group, Element->Rect, Element->Color);
                }
    
                if (Element->Flags & ui_flag::render_text) {
                    game_font* Font = Element->Font;
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
    Assert(UI.Tree.Parent);
    ui_axis OppositeAxis = Opposite(Axis);

    UI_Alignment Alignments[2];
    Alignments[Axis] = UI_Alignment_free;
    Alignments[OppositeAxis] = UI_Alignment_max;

    float ParentsSize[2]       = { UI.Tree.Parent->Rect.Width, UI.Tree.Parent->Rect.Height };
    float ParentsParentSize[2] = { (float)UI.Group->Width, (float)UI.Group->Height };
    if (UI.Tree.Parent->Parent) {
        ParentsSize[0] = UI.Tree.Parent->Parent->Rect.Width;
        ParentsSize[1] = UI.Tree.Parent->Parent->Rect.Height;
    }
    
    float SideBarSize = ParentsParentSize[Axis] / ParentsSize[Axis] * ParentsParentSize[Axis];
    UI_Size Size[2] = {};
    Size[Axis] = UISizeNull(SideBarSize);
    Size[OppositeAxis] = UISizePixels(SIDEBAR_WIDTH);
    UI_Element* Element = PushUIElement(
        "Sidebar",
        .AlignmentX = Alignments[axis_x],
        .AlignmentY = Alignments[axis_y],
        .Flags = ui_flag::render_rect,
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

    Element->Color = ChangeAlpha(color::WHITE, Alpha);

    Element->Parent->RelativePosition[Axis] = Element->Scroll * (ParentsParentSize[Axis] - ParentsSize[Axis]);
    Element->RelativePosition[Axis] = -Element->Parent->RelativePosition[Axis] + Element->Scroll * (ParentsParentSize[Axis] - SideBarSize);
}

struct ui_menu {
    UI_Element* Element;
    UI_Alignment Alignment;
    ui_axis StackAxis;

    ui_menu(
        const char* Text,
        UI_Element_options Options = {}
    ) {
        Options.Color = ChangeAlpha(color::BLACK, 0.7f);
        StackAxis = Options.Stack;
        Alignment = StackAxis == axis_x ? Options.AlignmentX : Options.AlignmentY;
        ui_axis NoStack = Opposite(StackAxis);
        Options.Flags |= ui_flag::render_rect;
        UI_Size Sizes[2] = {};
        if (StackAxis == axis_x) {
            Options.Flags |= ui_flag::stack_children_x;
            Sizes[axis_x] = UISizeSumChildren();
            Sizes[axis_y] = UISizeMaxChildren();
        }
        else {
            Options.Flags |= ui_flag::stack_children_y;
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
        if (Element->Parent) {
            ParentSize = axis_x ? Element->Parent->Rect.Width : Element->Parent->Rect.Height;
        }
        float ElementSize = StackAxis == axis_x ? Element->Rect.Width : Element->Rect.Height;
        if (ElementSize > ParentSize) {
            UISidebar(StackAxis);
            Element->Alignment[StackAxis] = UI_Alignment_free;
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
        game_font* Font = UI.Group->DebugFont;
        float Points = 12;
        float Width = 0, Height = 0;
        GetTextWidthAndHeight(Text, Font, Points, &Width, &Height);
        UI_Size Sizes[2] = {
            UISizeMaxChildren(Width),
            UISizeSumChildren(Height)
        };
        
        UI_Element* Element = PushUIElement(
            Text, 
            .AlignmentX = UI_Alignment_min, .AlignmentY = UI_Alignment_min,
            .Flags = ui_flag::render_text | ui_flag::stack_children_y,
            .Points = Points,
            .SizeX = Sizes[axis_x], .SizeY = Sizes[axis_y]
        );
        Element->Rect.Width = Width;
        Element->Rect.Height = Height;

        bool Hovered = IsIn(Element->Rect, UI.Input->Mouse.Cursor);
        if (Hovered) {
            Element->Color = color::YELLOW;
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
    UI_Element_options Options = {}
) {
    UI_Size Sizes[2];
    UISizeText(Text, Options.Points, Sizes);
    Options.Flags |= ui_flag::render_text;
    Options.SizeX = Sizes[axis_x];
    Options.SizeY = Sizes[axis_y];
    UI_Element* Element = PushUIElement(Text, Options);
}

#define UIText(Text, ...) _UIText(Text, { __VA_ARGS__ })

bool UIButton(const char* Text, float Points = 20.0f) {
    UI_Size Sizes[2];
    UISizeText(Text, Points, Sizes);
    UI_Element* Element = PushUIElement(
        Text, 
        .AlignmentX = UI_Alignment_center, .AlignmentY = UI_Alignment_center,
        .Flags = ui_flag::render_text,
        .Points = Points,
        .SizeX = Sizes[axis_x], .SizeY = Sizes[axis_y]
    );
    if (Element->Parent) {
        if (Element->Parent->Flags & ui_flag::stack_children_x) {
            Element->Alignment[0] = UI_Alignment_free;
        }
        if (Element->Parent->Flags & ui_flag::stack_children_y) {
            Element->Alignment[1] = UI_Alignment_free;
        }
    }

    bool Hovered = IsIn(Element->Rect, UI.Input->Mouse.Cursor);
    if (Hovered) {
        Element->Color = color::YELLOW;
    }

    return Element->Clicked;
}

void UIDebugvalue(debug_entry* Entry) {
    UI_Size Sizes[2] = {
        {UI_Size_pixels, 0},
        {UI_Size_pixels, 0},
    };
    UpdateAndSizeDebugEntry(UI.Group->DebugFont, Entry, &Sizes[axis_x].value, &Sizes[axis_y].value);

    UI_Element* Element = PushUIElement(
        Entry->Name.Content, 
        .AlignmentX = UI_Alignment_min,
        .SizeX = Sizes[axis_x], .SizeY = Sizes[axis_y]
    );
    Element->DebugEntry = Entry;
    if (IsStructType(Entry->Type)) {
        Element->Size[axis_x] = UISizeMaxChildren(Sizes[axis_x].value);
        Element->Size[axis_y] = UISizeSumChildren(Sizes[axis_y].value);
        Element->Flags = ui_flag::stack_children_y;
        if (Element->Hovered) Element->Color = color::YELLOW;
        if (Entry->value) {
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
                        uint8* Pointer = (uint8*)Entry->value + Member.Offset;
                        if (Member.IsPointer) {
                            debug_entry* ChildEntry = _AddDebugEntry(
                                UI.DebugInfo, 
                                Member.Name, 
                                Member.MemberType, 
                                Member.Size, 
                                Pointer ? *(void**)Pointer : nullptr, 
                                Entry->Editable, 
                                Entry
                            );
                            UIDebugvalue(ChildEntry);
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
                                UIDebugvalue(&ChildEntries[j]);
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
                            UIDebugvalue(ChildEntry);
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
    game_state* State = (game_state*)Memory->Permanent.Base;
    float Time = State->Time;
    debug_info* DebugInfo = &Memory->DebugInfo;
    game_entity* Camera = State->ActiveCamera;

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
            .AlignmentX = UI_Alignment_center, .AlignmentY = UI_Alignment_center,
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
        PushDebugVector(Group, 0.08 * Group->Height * XAxis, AxisOrigin, ChangeAlpha(color::RED, DebugAlpha));
        PushDebugVector(Group, 0.08 * Group->Height * YAxis, AxisOrigin, ChangeAlpha(color::GREEN, DebugAlpha));
        PushDebugVector(Group, 0.08 * Group->Height * ZAxis, AxisOrigin, ChangeAlpha(color::BLUE, DebugAlpha));

        // Debug camera basis
        // basis CameraBasis = GetCameraBasis(State->ActiveCamera->Angle, State->ActiveCamera->Pitch);
        // PushDebugVector(Group, CameraBasis, CameraBasis.X, V3(0,0,0), color::WHITE);
        // PushDebugVector(Group, CameraBasis, CameraBasis.Y, V3(0,0,0), color::WHITE);
        // PushDebugVector(Group, CameraBasis, CameraBasis.Z, V3(0,0,0), color::WHITE);
        // DEBUG_VALUE(CameraBasis.Z, v3);
        
        // Origin
        PushCircle(Group, V3(0, 0, 0), 0.05f, normalize(State->ActiveCamera->Transform.Translation), color::WHITE);
        
        ui_menu DebugMenu = UIMenu(
            "Debug Menu",
            .AlignmentX = UI_Alignment_min, .AlignmentY = UI_Alignment_min,
            .MarginX = 5.0f
        );

        int i = 0;
        int nEntries = DebugInfo->nEntries;
        for (; i < nEntries; i++) {
            debug_entry* Entry = &DebugInfo->Entries[i];
            UIDebugvalue(Entry);
        }

        if (UIDropdown(Arenas)) {
            DEBUG_VALUE(Memory->Transient, Arena);
            DEBUG_VALUE(Memory->Permanent, Arena);

            Arena* VertexArena = Group->VertexBuffer.Vertices;
            DEBUG_VALUE(VertexArena[vertex_layout_v2_id], Arena);
            DEBUG_VALUE(VertexArena[vertex_layout_v2_v2_id], Arena);
            DEBUG_VALUE(VertexArena[vertex_layout_v3_id], Arena);
            DEBUG_VALUE(VertexArena[vertex_layout_v3_v2_id], Arena);
            DEBUG_VALUE(VertexArena[vertex_layout_v3_v2_v3_id], Arena);
            DEBUG_VALUE(VertexArena[vertex_layout_bones_id], Arena);
            nEntries = DebugInfo->nEntries;
            for (; i < nEntries; i++) {
                debug_entry* Entry = &DebugInfo->Entries[i];
                UIDebugvalue(Entry);
            }
        }

        if (UIDropdown(Entities)) {
            game_entity** Entities = PushArray(&Memory->Transient, State->Entities.Count, game_entity*);
            uint32 nEntities = 0;
            uint32 Index = 0;
            while (nEntities < State->Entities.Count && Index < MAX_ENTITIES) {
                if (State->Entities.IsOccupied[Index]) {
                    Entities[nEntities++] = &State->Entities[Index++];
                }
                else {
                    Index++;
                    continue;
                }
            }
            DEBUG_POINTER_ARRAY(Entities, nEntities, game_entity);
            nEntries = DebugInfo->nEntries;
            for (; i < nEntries; i++) {
                debug_entry* Entry = &DebugInfo->Entries[i];
                UIDebugvalue(Entry);
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

    UI.Tree.Current = nullptr;
    UI.Tree.Parent = nullptr;

    Assert(UI.IDStack.n == 0);
}

#endif