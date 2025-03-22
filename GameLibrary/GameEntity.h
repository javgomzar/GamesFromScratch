#ifndef GAME_ENTITY
#define GAME_ENTITY

#include "GameMath.h"
#include "GamePlatform.h"
#include "GameAssets.h"
#include "GameInput.h"

/*
    TODO:
        - UI elements (maybe don't go crazy if possible).
*/

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Entities                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

enum game_entity_type {
    Character,
    Enemy,
    Camera,
    Prop,
    UIElement,

    game_entity_type_count
};

struct game_entity {
    char Name[32];
    int ID;
    int Index;
    game_entity_type Type;
    transform Transform;
    v3 Velocity;
    bool Active;
};

game_entity Entity(
    const char* Name,
    transform T = Transform(V3(0,0,0)),
    v3 Velocity = V3(0,0,0),
    bool Active = true
) {
    game_entity Result = {0};
    Result.ID = -1;
    strcpy_s(Result.Name, Name);
    Result.Transform = T;
    Result.Velocity = Velocity;
    Result.Active = Active;
    return Result;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Camera                                                                                                                                       |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct camera {
    int EntityID;
    basis Basis;
    v3 Position;
    float Distance;
    float Pitch;
    float Angle;
    matrix4 View;
};

basis GetCameraBasis(float Angle, float Pitch) {
    float cosA = cosf(Angle * Degrees);
    float sinA = sinf(Angle * Degrees);
    float cosP = cosf(Pitch * Degrees);
    float sinP = sinf(Pitch * Degrees);

    v3 X = V3(        cosA,  0.0,         sinA);
    v3 Y = V3(-sinA * sinP, cosP,  cosA * sinP);
    v3 Z = V3( sinA * cosP, sinP, -cosA * cosP);

    basis Result;
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    return Result;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Colliders                                                                                                                                    |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

enum collider_type {
    Rect_Collider,
    Cube_Collider,
    Sphere_Collider
};

struct collider {
    collider_type Type;
    v3 Center;
    scale Scale;
};

bool Collide(collider Collider, v3 Position) {
    switch(Collider.Type) {
        case Rect_Collider: {
            return fabs(Position.X - Collider.Center.X) < Collider.Scale.X / 2.0f &&
                   fabs(Position.Y - Collider.Center.Y) < Collider.Scale.Y / 2.0f;
        } break;

        case Cube_Collider: {
            return fabs(Position.X - Collider.Center.X) < Collider.Scale.X / 2.0 &&
                   fabs(Position.Y - Collider.Center.Y) < Collider.Scale.Y / 2.0 &&
                   fabs(Position.Z - Collider.Center.Z) < Collider.Scale.Z / 2.0;
        } break;

        case Sphere_Collider: {
            return modulus(Position - Collider.Center) < Collider.Scale.X;
        } break;
    }
    return false;
}

/*
Fast Ray-Box Intersection
by Andrew Woo
from "Graphics Gems", Academic Press, 1990
*/
bool HitBoundingBox(double minB[3], double maxB[3], double origin[3], double dir[3], double coord[3])
/* double minB[NUMDIM], maxB[NUMDIM];		box */
/* double origin[NUMDIM], dir[NUMDIM];		ray */
/* double coord[NUMDIM];			hit point   */
{
    bool inside = true;
    char quadrant[3];
    register int i;
    int whichPlane;
    double maxT[3];
    double candidatePlane[3];
    char LEFT = 1;
    char RIGHT = 0;
    char MIDDLE = 2;

    /* Find candidate planes; this loop can be avoided if
    rays cast all from the eye(assume perpsective view) */
    for (i = 0; i < 3; i++)
        if (origin[i] < minB[i]) {
            quadrant[i] = LEFT;
            candidatePlane[i] = minB[i];
            inside = false;
        }
        else if (origin[i] > maxB[i]) {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxB[i];
            inside = false;
        }
        else {
            quadrant[i] = MIDDLE;
        }

    /* Ray origin inside bounding box */
    if (inside) {
        coord = origin;
        return true;
    }


    /* Calculate T distances to candidate planes */
    for (i = 0; i < 3; i++)
        if (quadrant[i] != MIDDLE && dir[i] != 0.)
            maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
        else
            maxT[i] = -1.;

    /* Get largest of the maxT's for final choice of intersection */
    whichPlane = 0;
    for (i = 1; i < 3; i++)
        if (maxT[whichPlane] < maxT[i])
            whichPlane = i;

    /* Check final candidate actually inside box */
    if (maxT[whichPlane] < 0.) return false;
    for (i = 0; i < 3; i++)
        if (whichPlane != i) {
            coord[i] = origin[i] + maxT[whichPlane] * dir[i];
            if (coord[i] < minB[i] || coord[i] > maxB[i])
                return false;
        }
        else {
            coord[i] = candidatePlane[i];
        }
    return true;				/* ray hits box */
}

bool Raycast(v3 Origin, v3 Direction, collider Collider) {
    Assert(Collider.Type == Cube_Collider);
    double minB[3] = { 0 };
    minB[0] = Collider.Center.X - Collider.Scale.X / 2.0;
    minB[1] = Collider.Center.Y - Collider.Scale.Y / 2.0;
    minB[2] = Collider.Center.Z - Collider.Scale.Z / 2.0;
    double maxB[3] = { 0 };
    maxB[0] = Collider.Center.X + Collider.Scale.X / 2.0;
    maxB[1] = Collider.Center.Y + Collider.Scale.Y / 2.0;
    maxB[2] = Collider.Center.Z + Collider.Scale.Z / 2.0;
    double origin[3] = { Origin.X, Origin.Y, Origin.Z };
    double dir[3] = { Direction.X, Direction.Y, Direction.Z };
    double coord[3] = { 0,0,0 };

    return HitBoundingBox(minB, maxB, origin, dir, coord);
}

bool Raycast(camera* Camera, v3 CameraPosition, double Width, double Height, v2 Mouse, collider Collider) {
    Assert(Collider.Type == Cube_Collider);
    v3 ScreenOffset =
        (2.0 * Mouse.X / Width - 1.0) *    Camera->Basis.X +
        (Height - 2.0 * Mouse.Y) / Width * Camera->Basis.Y - 
                                           Camera->Basis.Z;
    return Raycast(CameraPosition + Camera->Distance * Camera->Basis.Z, ScreenOffset, Collider);
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Character                                                                                                                                    |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct character {
    int EntityID;
    int MaxHP;
    int HP;
    armature Armature;
    game_animator Animator;
};

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Enemies                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct enemy {
    int EntityID;
    int MaxHP;
    int HP;
};

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Props                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct prop {
    int EntityID;
    game_mesh_id MeshID;
    color Color;
};

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | User Interface                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

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
    int EntityID;
    int ParentIndex;
    v2 Offset;
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

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Entity List                                                                                                                                  |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_CAMERAS = 10;
const int MAX_CHARACTERS = 1;
const int MAX_ENEMIES = 32;
const int MAX_PROPS = 32;
const int MAX_ENTITIES = MAX_CAMERAS + MAX_CHARACTERS + MAX_ENEMIES;
struct game_entity_list {
    game_assets* Assets;
    int nFreeIDs;
    int FreeIDs[MAX_ENTITIES];
    int nEntities;
    game_entity Entities[MAX_ENTITIES];
    int nCameras;
    camera Cameras[MAX_CAMERAS];
    int nCharacters;
    character Character[MAX_CHARACTERS];
    int nEnemy;
    enemy Enemy[MAX_ENEMIES];
    int nProps;
    prop Prop[MAX_PROPS];
};

int AddEntity(
    game_entity_list* List,
    const char* Name,
    game_entity_type Type,
    v3 Position,
    quaternion Rotation,
    scale Scale,
    bool Active
) {
    Assert(List->nEntities < MAX_ENTITIES);
    transform T = Transform(Position, Rotation, Scale);

    // If any ID is free, use it
    int EntityID = 0;
    if (List->nFreeIDs > 0) {
        EntityID = List->FreeIDs[List->nFreeIDs - 1];
        List->FreeIDs[List->nFreeIDs-- - 1] = -1;
        List->nEntities++;
    }
    else {
        EntityID = List->nEntities++;
    }

    game_entity* Entity = &List->Entities[EntityID];
    Entity->ID = EntityID;
    Entity->Type = Type;
    Entity->Transform = T;
    Entity->Active = Active;
    strcpy_s(Entity->Name, Name);

    return EntityID;
}

void RemoveEntity(game_entity_list* List, int EntityID) {
    Assert(List->nEntities > EntityID);
    List->Entities[EntityID] = {0};
    List->nEntities--;
    List->FreeIDs[List->nFreeIDs] = EntityID;
    List->nFreeIDs++;
}

game_entity* QueryEntity(game_entity_list* List, game_entity_type Type, bool Active = true) {
    for (int i = 0; i < List->nEntities; i++) {
        game_entity* Entity = &List->Entities[i];
        if (Entity->Type == Type && Entity->Active == Active) {
            return Entity;
        }
    }
    return 0;
}

int QueryEntityCount(game_entity_list* List, game_entity_type Type, bool Active = true) {
    int Result = 0;
    for (int i = 0; i < List->nEntities; i++) {
        game_entity* Entity = &List->Entities[i];
        if (Entity->Type == Type && Entity->Active == Active) {
            Result++;
        }
    }
    return Result;
}

// Entity initialization ___________________________________________________________________________________________________________________

void AddCamera(
    game_entity_list* List, 
    v3 Position = V3(0,0,0), 
    float Angle = 45.0, float Pitch = 45.0, 
    float Distance = 9.0
) {
    int CameraIndex = List->nCameras++;
    camera* Cam = &List->Cameras[CameraIndex];
    Cam->Angle = Angle;
    Cam->Pitch = Pitch;
    Cam->Position = Position;
    Cam->Distance = Distance;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Camera %d", CameraIndex);

    quaternion Rotation = Quaternion(Cam->Angle * Degrees, V3(0,1,0)) * Quaternion(Cam->Pitch * Degrees, V3(1,0,0));
    int EntityID = AddEntity(List, NameBuffer, Camera, Position, Rotation, Scale(), CameraIndex == 0);
    game_entity* CameraEntity = &List->Entities[EntityID];
    CameraEntity->Index = CameraIndex;
    Cam->EntityID = EntityID;
}

void AddCharacter(game_entity_list* List, v3 Position, int MaxHP) {
    int CharacterIndex = List->nCharacters++;
    character* pCharacter = &List->Character[CharacterIndex];
    pCharacter->MaxHP = MaxHP;
    pCharacter->HP = MaxHP;
    pCharacter->Animator.Active = false;
    pCharacter->Animator.Animation = GetAsset(List->Assets, Animation_Walking_ID);
    game_mesh* Mesh = GetAsset(List->Assets, Mesh_Body_ID);
    pCharacter->Armature = Mesh->Armature;
    pCharacter->Animator.Armature = &pCharacter->Armature;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Character %d", CharacterIndex);

    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
    int EntityID = AddEntity(List, NameBuffer, Character, Position, Rotation, Scale(), true);
    game_entity* CharacterEntity = &List->Entities[EntityID];
    CharacterEntity->Index = CharacterIndex;
    pCharacter->EntityID = EntityID;
}

void AddEnemy(game_entity_list* List, v3 Position, int MaxHP) {
    int EnemyIndex = List->nEnemy++;
    enemy* pEnemy = &List->Enemy[EnemyIndex];
    pEnemy->MaxHP = MaxHP;
    pEnemy->HP = MaxHP;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Enemy %d", EnemyIndex);

    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
    int EntityID = AddEntity(List, NameBuffer, Enemy, Position, Rotation, Scale(), true);
    game_entity* EnemyEntity = &List->Entities[EntityID];
    EnemyEntity->Index = EnemyIndex;
    pEnemy->EntityID = EntityID;
}

void AddUIElement(
    game_entity_list* List,
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

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Game state                                                                                                                                   |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct game_state {
    memory_arena RenderArena;
    memory_arena StringsArena;
    memory_arena GeneralPurposeArena;
    memory_arena TransientArena;
    game_entity_list EntityList;
    double dt;
    double Time;
};

void Update(camera** pActiveCamera, game_state* State, game_input* Input) {
    game_entity_list* List = &State->EntityList;
    
// Cameras _________________________________________________________________________________________________________________________________
    camera* ActiveCamera = 0;
    for (int i = 0; i < List->nCameras; i++) {
        camera* Cam = &List->Cameras[i];
        game_entity* CamEntity = &List->Entities[Cam->EntityID];
        if(CamEntity->Active) {
            *pActiveCamera = Cam;
            ActiveCamera = Cam;
        // Zoom
            if (Input->Mouse.Wheel > 0)      Cam->Distance /= 1.2;
            else if (Input->Mouse.Wheel < 0) Cam->Distance *= 1.2;

        // Orbit around position
            if (Input->Mouse.MiddleClick.IsDown && Input->Mouse.MiddleClick.WasDown) {
                v3 Offset = Input->Mouse.Cursor - Input->Mouse.LastCursor;
                double AngularVelocity = 0.5;

                Cam->Angle -= AngularVelocity * Offset.X;
                Cam->Pitch += AngularVelocity * Offset.Y;
            }

            v2 Joystic = V2(Input->Controller.RightJoystick.X, Input->Controller.RightJoystick.Y);

            if (modulus(Joystic) > 0.1) {
                Cam->Angle -= 3.0 * Joystic.X;
                Cam->Pitch -= 3.0 * Joystic.Y;
            }

        // Translation
            CamEntity->Velocity = V3(0, 0, 0);
            if      (Input->Keyboard.D.IsDown)     CamEntity->Velocity.X += 1.0;
            else if (Input->Keyboard.A.IsDown)     CamEntity->Velocity.X -= 1.0;
            if      (Input->Keyboard.W.IsDown)     CamEntity->Velocity.Z += 1.0;
            else if (Input->Keyboard.S.IsDown)     CamEntity->Velocity.Z -= 1.0;
            if      (Input->Keyboard.Space.IsDown) CamEntity->Velocity.Y += 1.0;
            else if (Input->Keyboard.Shift.IsDown) CamEntity->Velocity.Y -= 1.0;

        // Rotation
            Cam->Basis = GetCameraBasis(Cam->Angle, Cam->Pitch);
            quaternion Rotation = Quaternion(Cam->Angle * Degrees, V3(0,1,0)) * Quaternion(Cam->Pitch * Degrees, V3(-1,0,0));
            transform Test = Transform(Rotation);
            matrix4 MatrixT = Matrix(Test);

            basis HorizontalBasis = GetCameraBasis(Cam->Angle, 0);

            v3 Direction = normalize(CamEntity->Velocity);
            float Speed = 2 * Cam->Distance;
            CamEntity->Velocity = Speed * (Direction.Y * V3(0.0, 1.0, 0.0) + Direction.X * HorizontalBasis.X - Direction.Z * HorizontalBasis.Z);
            Cam->Position += State->dt * CamEntity->Velocity;

            CamEntity->Transform = Transform(V3(0,0,Cam->Distance) - Cam->Position * Cam->Basis, Rotation);
        }
    }

// Character _______________________________________________________________________________________________________________________________
    for (int i = 0; i < List->nCharacters; i++) {
        character* Character = &List->Character[i];
        game_entity* CharacterEntity = &List->Entities[Character->EntityID];
        CharacterEntity->Transform.Translation = V3(ActiveCamera->Position.X, 0, ActiveCamera->Position.Z);
        CharacterEntity->Transform.Rotation = Quaternion(ActiveCamera->Angle * Degrees, V3(0,1,0));

        if (Input->Keyboard.W.IsDown || Input->Keyboard.A.IsDown || Input->Keyboard.S.IsDown || Input->Keyboard.D.IsDown) {
            Character->Animator.Active = true;
        }
        else {
            Character->Animator.Active = false;
        }

        Update(&Character->Animator);
    }

// Enemies _________________________________________________________________________________________________________________________________
    for (int i = 0; i < List->nEnemy; i++) {
        enemy* pEnemy = &List->Enemy[i];
        game_entity* EnemyEntity = &List->Entities[pEnemy->EntityID];
        EnemyEntity->Transform.Translation = V3(0,3.2 + sin(3 * State->Time), 0);
    }

// User interface __________________________________________________________________________________________________________________________
    // for (int i = 0; i < List->nUIElements; i++) {

    // }
}

#endif