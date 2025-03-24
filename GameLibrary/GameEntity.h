#ifndef GAME_ENTITY
#define GAME_ENTITY

#include "GameMath.h"
#include "GamePlatform.h"
#include "GameAssets.h"
#include "GameInput.h"


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
    int ParentID;
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

#define DefineEntityList(maxNumber, type) struct type##_list {int nFreeIDs; int FreeIDs[maxNumber]; int Size; type List[maxNumber];}

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

const int MAX_CAMERAS = 10;
DefineEntityList(MAX_CAMERAS, camera);

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
    bool Jumping;
};

const int MAX_CHARACTERS = 1;
DefineEntityList(MAX_CHARACTERS, character);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Enemies                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct enemy {
    int EntityID;
    int MaxHP;
    int HP;
};

const int MAX_ENEMIES = 32;
DefineEntityList(MAX_ENEMIES, enemy);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Props                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct prop {
    int EntityID;
    game_mesh_id MeshID;
    color Color;
};

const int MAX_PROPS = 32;
DefineEntityList(MAX_PROPS, prop);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Entity List                                                                                                                                  |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_ENTITIES = MAX_CAMERAS + MAX_CHARACTERS + MAX_ENEMIES + MAX_PROPS;
struct game_entity_list {
    game_assets* Assets;
    int nFreeIDs;
    int FreeIDs[MAX_ENTITIES];
    int nEntities;
    game_entity Entities[MAX_ENTITIES];
    camera_list Cameras;
    character_list Characters;
    enemy_list Enemies;
    prop_list Props;
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
    int EntityID = -1;
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
    Assert(List->Cameras.Size < MAX_CAMERAS);
    // If any ID is free, use it
    int CameraID = -1;
    if (List->Cameras.nFreeIDs > 0) {
        CameraID = List->Cameras.FreeIDs[List->Cameras.nFreeIDs - 1];
        List->FreeIDs[List->nFreeIDs-- - 1] = -1;
        List->Cameras.Size++;
    }
    else CameraID = List->Cameras.Size++;

    camera* Cam = &List->Cameras.List[CameraID];
    Cam->Angle = Angle;
    Cam->Pitch = Pitch;
    Cam->Position = Position;
    Cam->Distance = Distance;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Camera %d", CameraID);

    quaternion Rotation = Quaternion(Cam->Angle * Degrees, V3(0,1,0)) * Quaternion(Cam->Pitch * Degrees, V3(1,0,0));
    int EntityID = AddEntity(List, NameBuffer, Camera, Position, Rotation, Scale(), CameraID == 0);
    Cam->EntityID = EntityID;
    game_entity* CameraEntity = &List->Entities[EntityID];
    CameraEntity->Index = CameraID;
}

void AddCharacter(game_entity_list* List, v3 Position, int MaxHP) {
    Assert(List->Characters.Size < MAX_CHARACTERS);
    // If any ID is free, use it
    int CharacterID = -1;
    if (List->Characters.nFreeIDs > 0) {
        CharacterID = List->Characters.FreeIDs[List->Characters.nFreeIDs - 1];
        List->FreeIDs[List->nFreeIDs-- - 1] = -1;
        List->Characters.Size++;
    }
    else CharacterID = List->Characters.Size++;

    character* pCharacter = &List->Characters.List[CharacterID];
    pCharacter->MaxHP = MaxHP;
    pCharacter->HP = MaxHP;
    pCharacter->Animator.Active = false;
    pCharacter->Animator.Animation = GetAsset(List->Assets, Animation_Walking_ID);
    game_mesh* Mesh = GetAsset(List->Assets, Mesh_Body_ID);
    pCharacter->Armature = Mesh->Armature;
    pCharacter->Animator.Armature = &pCharacter->Armature;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Character %d", CharacterID);

    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
    int EntityID = AddEntity(List, NameBuffer, Character, Position, Rotation, Scale(), true);
    pCharacter->EntityID = EntityID;
    game_entity* CharacterEntity = &List->Entities[EntityID];
    CharacterEntity->Index = CharacterID;
}

void AddEnemy(game_entity_list* List, v3 Position, int MaxHP) {
    Assert(List->Characters.Size < MAX_ENEMIES);
    // If any ID is free, use it
    int EnemyID = -1;
    if (List->Enemies.nFreeIDs > 0) {
        EnemyID = List->Enemies.FreeIDs[List->Enemies.nFreeIDs - 1];
        List->FreeIDs[List->nFreeIDs-- - 1] = -1;
        List->Enemies.Size++;
    }
    else EnemyID = List->Enemies.Size++;

    enemy* pEnemy = &List->Enemies.List[EnemyID];
    pEnemy->MaxHP = MaxHP;
    pEnemy->HP = MaxHP;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Enemy %d", EnemyID);

    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
    int EntityID = AddEntity(List, NameBuffer, Enemy, Position, Rotation, Scale(), true);
    pEnemy->EntityID = EntityID;
    game_entity* EnemyEntity = &List->Entities[EntityID];
    EnemyEntity->Index = EnemyID;
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

    for (int i = 0; i < List->Cameras.Size; i++) {
        camera* Cam = &List->Cameras.List[i];
        game_entity* CamEntity = &List->Entities[Cam->EntityID];
        if (CamEntity->Active) {
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

        // Rotation
            Cam->Basis = GetCameraBasis(Cam->Angle, Cam->Pitch);
            quaternion Rotation = Quaternion(Cam->Angle * Degrees, V3(0,1,0)) * Quaternion(Cam->Pitch * Degrees, V3(-1,0,0));
            transform Test = Transform(Rotation);
            matrix4 MatrixT = Matrix(Test);

            break;
        }
    }
        
// Characters ______________________________________________________________________________________________________________________________
    v3 CharacterPosition = V3(0,0,0);
    for (int i = 0; i < List->Characters.Size; i++) {
        character* Character = &List->Characters.List[i];
        game_entity* CharacterEntity = &List->Entities[Character->EntityID];
        
        CharacterEntity->Velocity = V3(0, 0, 0);
        if (Input->Keyboard.Space.IsDown && !Input->Keyboard.Space.WasDown) {
            if (!Character->Jumping){ 
                Character->Jumping = true;
                Character->Animator.Active = true;
                Character->Animator.Animation = GetAsset(List->Assets, Animation_Jumping_ID);
                Character->Animator.Loop = false;
                Character->Animator.CurrentFrame = 0;
            }
        }
        if (Character->Jumping && !Character->Animator.Active) {
            Character->Animator.Animation = GetAsset(List->Assets, Animation_Walking_ID);
            Character->Jumping = false;
        }
        
        if (Input->Keyboard.W.IsDown || Input->Keyboard.A.IsDown || Input->Keyboard.S.IsDown || Input->Keyboard.D.IsDown) {
        // Translation
            if      (Input->Keyboard.D.IsDown) CharacterEntity->Velocity.X += 1.0;
            else if (Input->Keyboard.A.IsDown) CharacterEntity->Velocity.X -= 1.0;
            if      (Input->Keyboard.W.IsDown) CharacterEntity->Velocity.Z += 1.0;
            else if (Input->Keyboard.S.IsDown) CharacterEntity->Velocity.Z -= 1.0;

            basis HorizontalBasis = GetCameraBasis(ActiveCamera->Angle, 0);
            v3 Direction = normalize(CharacterEntity->Velocity);
            float Speed = 20;
            CharacterEntity->Velocity = Speed * (Direction.Y * V3(0.0, 1.0, 0.0) + Direction.X * HorizontalBasis.X - Direction.Z * HorizontalBasis.Z);
            
            CharacterEntity->Transform.Rotation = Quaternion(ActiveCamera->Angle * Degrees, V3(0,1,0));

            if (!Character->Jumping) {
                Character->Animator.Active = true;
                Character->Animator.Loop = true;
            }
        }
        else if (!Character->Jumping) {
            Character->Animator.Active = false;
        }
        CharacterEntity->Transform.Translation += State->dt * CharacterEntity->Velocity;
        CharacterPosition = CharacterEntity->Transform.Translation;

        Update(&Character->Animator);
    }
    
// Autofollow player _______________________________________________________________________________________________________________________
    ActiveCamera->Position = CharacterPosition + V3(0,3.2,0);
    game_entity* ActiveCameraEntity = &List->Entities[ActiveCamera->EntityID];
    ActiveCameraEntity->Transform.Translation = V3(0,0,ActiveCamera->Distance) - ActiveCamera->Position * ActiveCamera->Basis;

// Enemies _________________________________________________________________________________________________________________________________
    for (int i = 0; i < List->Enemies.Size; i++) {
        enemy* pEnemy = &List->Enemies.List[i];
        game_entity* EnemyEntity = &List->Entities[pEnemy->EntityID];
        EnemyEntity->Transform.Translation = V3(0,3.2 + sin(3 * State->Time), 0);
    }
}

#endif