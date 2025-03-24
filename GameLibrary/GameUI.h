#ifndef GAME_UI
#define GAME_UI

#include "GameMath.h"
#include "GamePlatform.h"
#include "GameAssets.h"
#include "GameInput.h"


enum ui_element_type {
    UI_Slider,
    UI_Menu,
    UI_HealthBar,
    UI_Button,
    UI_Text,
    UI_Graph,

    ui_element_type_count
};

enum ui_alignment {
    Centered,
    Left,
    Top,
    Right,
    Bottom
};

struct ui_element {
    ui_element_type Type;
    ui_alignment Alignment;
    collider Collider;
    int Index;
    int ParentIndex;
    v2 Position;
    double Width;
    double Height;
    color Color;
    bool Clicked;
    bool Hovered;
    bool Dragged;
};

struct ui_slider {
    double Value;
    double MinValue;
    double MaxValue;
};

/* 
    Updates Slider. If slider is clicked and dragged, the value of the slider will fluctuate accordingly. Min and max values
    of slider will be enforced.
*/
void Update(ui_slider* Slider, game_input* Input, collider* Collider, v3 Position) {
    if (
        Input->Mouse.LeftClick.IsDown && 
        Collide(*Collider, V3(Input->Mouse.Cursor.X, Input->Mouse.Cursor.Y, 0.0))
    ) {
        double Range = (Slider->MaxValue - Slider->MinValue) / 60.0;
        Slider->Value = Slider->MaxValue - Range * (Input->Mouse.Cursor.Y - Position.Y);

        // Min and max values shall not be surpassed
        if (Slider->Value < Slider->MinValue) Slider->Value = Slider->MinValue;
        else if (Slider->Value > Slider->MaxValue) Slider->Value = Slider->MaxValue;
    }
};

typedef void on_click();

void OnClickStub() {};

struct ui_button {

    on_click OnClick;
};

struct ui_text {

};

struct ui_menu_item {
    char Text[32];
    v3 Offset;
    on_click OnClick;
};

const int MAX_MENU_ITEMS = 10;
struct ui_menu {
    int nItems;
    ui_menu_item Items[MAX_MENU_ITEMS];
};

struct ui_healthbar {
    int HP;
    int MaxHP;
    char Text[32];
};

const int MAX_SLIDERS = 5;
const int MAX_MENUS = 5;
const int MAX_TEXTS = 5;
const int MAX_BUTTONS = 5;
const int MAX_HEALTHBARS = 5;
struct user_interface {
    int nSliders;
    ui_slider Slider[MAX_SLIDERS];
    int nMenus;
    ui_menu Menu[MAX_MENUS];
    int nTexts;
    ui_text Text[MAX_TEXTS];
    int nHealthBars;
    ui_healthbar HealthBar[MAX_HEALTHBARS];
    int nButtons;
    ui_button Button[MAX_BUTTONS];
};

void AddUIElement(
    user_interface* UI,
    ui_element_type Type,
    int ParentIndex,
    v2 Offset,
    double Width,
    double Height,
    color Color
) {

}

void AddSlider(user_interface* UI, v3 Position, double MinValue = 0.0, double MaxValue = 1.0, color Color = Black) {
    int SliderIndex = UI->nSliders++;
    ui_slider* Slider = &UI->Slider[SliderIndex];
    
    // Slider->Position = Position;
    // Slider->MinValue = MinValue;
    // Slider->MaxValue = MaxValue;
    // Slider->Value = 0.5 * (MinValue + MaxValue);    

    // Slider->Color = Color;
}

#endif