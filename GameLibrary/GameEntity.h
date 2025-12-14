#ifndef GAME_ENTITY
#define GAME_ENTITY

#include "GameMath.h"
#include "GamePlatform.h"
#include "GameAssets.h"
#include "GameInput.h"
#include "GameRender.h"
#include "Particles.h"

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Entities                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(game_entity_type,
    Entity_Type_Character,
    Entity_Type_Enemy,
    Entity_Type_Camera,
    Entity_Type_Prop,
    Entity_Type_Weapon
);

INTROSPECT
struct game_entity {
    char Name[32];
    int ID;
    game_entity* Parent;
    int Index;
    game_entity_type Type;
    transform Transform;
    v3 Velocity;
    collider Collider;
    bool Collided;
    bool Active;
    bool Hovered;
};

game_entity Entity(
    const char* Name,
    transform T = Transform(V3(0,0,0)),
    v3 Velocity = V3(0,0,0),
    collider Collider = SphereCollider(V3(0.0f, 0.0f, 0.0f), 1.0f),
    bool Active = true
) {
    game_entity Result = {0};
    Result.ID = -1;
    strcpy_s(Result.Name, Name);
    Result.Transform = T;
    Result.Velocity = Velocity;
    Result.Active = Active;
    Result.Collider = Collider;
    return Result;
}

bool Collide(game_entity* Entity1, game_entity* Entity2) {
    collider Collider1 = Entity1->Collider;
    Collider1.Offset += Entity1->Transform.Translation;
    if (Collider1.Type == Capsule_Collider) {
        Collider1.Capsule.Segment = Entity1->Transform * Collider1.Capsule.Segment;
    }
    collider Collider2 = Entity2->Collider;
    Collider2.Offset += Entity2->Transform.Translation;
    if (Collider2.Type == Capsule_Collider) {
        Collider2.Capsule.Segment = Entity2->Transform * Collider2.Capsule.Segment;
    }
    if (Collide(Collider1, Collider2)) {
        Entity1->Collided = true;
        Entity2->Collided = true;
        return true;
    }
    return false;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Camera                                                                                                                                       |
// +----------------------------------------------------------------------------------------------------------------------------------------------+s

struct camera {
    uint32 ID;
    game_entity* Entity;
    game_entity* Follow;
    v3 Position;
    float Distance;
    float Pitch;
    float Angle;
    basis Basis;
    matrix4 View;
    bool OnAir;
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

matrix4 GetViewMatrix(camera* Camera) {
	matrix3 Basis = Camera->Basis;
	Basis.Z = -Basis.Z;
	Basis = transpose(Basis);

	v3 Translation = V3(0,0,Camera->Distance) - Camera->Position * Basis;
	matrix4 Result;
	Result.X = V4(Basis.X, 0);
	Result.Y = V4(Basis.Y, 0);
	Result.Z = V4(Basis.Z, 0);
	Result.W = V4(Translation, 1);

	return Result;
}

v2 GetScreenPosition(float ScreenWidth, float ScreenHeight, camera* Camera, v3 WorldPosition) {
    v4 Point = V4(WorldPosition, 1);
    matrix4 Projection = GetWorldProjectionMatrix(ScreenWidth, ScreenHeight);
    v4 ViewPoint = Point * Camera->View * Projection;
    float Factor = ViewPoint.W == 0 ? 0 : 1.0f/ViewPoint.W;
    v2 DevicePoint = Factor * V2(ViewPoint.X, ViewPoint.Y);
    v2 ScreenPoint = V2(
        0.5f * (1.0f + DevicePoint.X) * ScreenWidth,
        0.5f * (1.0f - DevicePoint.Y) * ScreenHeight
    );
    return ScreenPoint;
}

const int MAX_CAMERAS = 16;
DefineFreeList(MAX_CAMERAS, camera);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Enemies                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct enemy {
    uint32 ID;
    game_entity* Entity;
};

const int MAX_ENEMIES = 32;
DefineFreeList(MAX_ENEMIES, enemy);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Weapons                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(weapon_type,
    Weapon_Sword,
    Weapon_Shield
);

struct weapon {
    uint32 ID;
    weapon_type Type;
    color Color;
    game_entity* Entity;
    int ParentBone;
};

transform WeaponTransforms[weapon_type_count] = {
    Transform(
        V3(0.5f,2.0f,0),
        Quaternion(-0.25f * Tau, V3(0,1,0)) * Quaternion(-0.25f * Tau, V3(1,0,0))
    ),
    Transform(
        V3(-0.7f,2.2f,0),
        Quaternion(0.5f * Tau, V3(0,0,1)) * Quaternion(0.25f * Tau, V3(1,0,0))
    ),
};

const int MAX_WEAPONS = 32;
DefineFreeList(MAX_WEAPONS, weapon);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Character                                                                                                                                    |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(character_action_id,
    Character_Action_Idle_ID,
    Character_Action_Walk_ID,
    Character_Action_Jump_ID,
    Character_Action_Attack_ID
);

struct character_action {
    character_action_id ID;
    game_animation_id AnimationID;
    bool Loop;
};

enum character_class {
    Knight_Class,
    Rogue_Class,
    Hunter_Class,
    Wizard_Class,
    Bard_Class,
    Priest_Class
};

struct character {
    uint32 ID;
    armature Armature;
    game_animator Animator;
    game_entity* Entity;
    weapon* LeftHand;
    weapon* RightHand;
    character_action Action;
};

character_action CharacterAction(character_action_id ID) {
    character_action Result = {};
    Result.ID = ID;

    switch(ID) {
        case Character_Action_Idle_ID: {
            Result.AnimationID = Animation_Idle_ID;
            Result.Loop = true;
        } break;
        case Character_Action_Walk_ID: {
            Result.AnimationID = Animation_Walk_ID;
            Result.Loop = true;
        } break;
        case Character_Action_Jump_ID: {
            Result.AnimationID = Animation_Jump_ID;
        } break;
        case Character_Action_Attack_ID: {
            Result.AnimationID = Animation_Attack_ID;
        } break;
        default: Assert(false);
    }

    return Result;
}

character_action GetCharacterAction(character* Character, game_input* Input) {
    bool JumpingInput = Input->Mode == Keyboard && Input->Keyboard.Space.JustPressed ||
                        Input->Mode == Controller && Input->Controller.BButton.JustPressed;

    bool AttackInput = Input->Mode == Keyboard && Input->Keyboard.E.JustPressed ||
                       Input->Mode == Controller && Input->Controller.XButton.JustPressed;

    bool KeyboardMoving = Input->Keyboard.W.IsDown != Input->Keyboard.S.IsDown ||
                          Input->Keyboard.A.IsDown != Input->Keyboard.D.IsDown;
    bool ControllerMoving = fabs(Input->Controller.LeftJoystick.X) > 0.1 || fabs(Input->Controller.LeftJoystick.Y) > 0.1;
    bool MovingInput = (Input->Mode == Keyboard && KeyboardMoving) ||
                       (Input->Mode == Controller && ControllerMoving);

    character_action Result = Character->Action;
    if (AttackInput) {
        OutputDebugStringA("A");
    }

    switch(Character->Action.ID) {
        case Character_Action_Idle_ID: {
            Character->Animator.Active = true;
            if (JumpingInput || MovingInput || AttackInput) {
                Character->Animator.CurrentFrame = 0;
            }

            if     (JumpingInput) Result = CharacterAction(Character_Action_Jump_ID);
            else if (MovingInput) Result = CharacterAction(Character_Action_Walk_ID);
            else if (AttackInput) Result = CharacterAction(Character_Action_Attack_ID);
        } break;
        case Character_Action_Walk_ID: {
            if (JumpingInput) {
                Result = CharacterAction(Character_Action_Jump_ID);
                Character->Animator.CurrentFrame = 0;
            }
            else if (AttackInput) {
                Result = CharacterAction(Character_Action_Attack_ID);
                Character->Animator.CurrentFrame = 0;
            }
            else if (!MovingInput) {
                Character->Animator.Active = false;
                Result = CharacterAction(Character_Action_Idle_ID);
            }
        } break;
        case Character_Action_Jump_ID: {
            if (!Character->Animator.Active) {
                Result = CharacterAction(Character_Action_Idle_ID);
                Character->Animator.CurrentFrame = 0;
            }
        } break;
        case Character_Action_Attack_ID: {
            if (!Character->Animator.Active) {
                Result = CharacterAction(Character_Action_Idle_ID);
                Character->Animator.CurrentFrame = 0;
            }
        } break;
        default: Raise("Invalid character action");
    }

    Character->Animator.Loop = Result.Loop;
    return Result;
}

void Equip(weapon* Weapon, character* Character) {
    Weapon->Entity->Parent = Character->Entity;
    if (Weapon->Type == Weapon_Sword) {
        Character->RightHand = Weapon;
        Weapon->ParentBone = 8;
    }
    else if (Weapon->Type == Weapon_Shield) {
        Character->LeftHand = Weapon;
        Weapon->ParentBone = 2;
    }
}

const int MAX_CHARACTERS = 8;
DefineFreeList(MAX_CHARACTERS, character);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Props                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct prop {
    uint32 ID;
    game_mesh_id MeshID;
    game_bitmap_id Texture;
    color Color;
    game_entity* Entity;
};

const int MAX_PROPS = 32;
DefineFreeList(MAX_PROPS, prop);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Entity List                                                                                                                                  |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const int32 MAX_ENTITIES = MAX_CAMERAS + MAX_CHARACTERS + MAX_ENEMIES + MAX_PROPS;
DefineFreeList(MAX_ENTITIES, game_entity);

struct game_entity_state {
    game_entity_list Entities;
    camera_list Cameras;
    character_list Characters;
    enemy_list Enemies;
    prop_list Props;
    weapon_list Weapons;
};

game_entity* AddEntity(
    game_entity_state* State,
    const char* Name,
    game_entity_type Type,
    collider Collider,
    v3 Position = V3(0,0,0),
    quaternion Rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f),
    scale S = Scale(),
    bool Active = true
) {
    Assert(State->Entities.Count < MAX_ENTITIES);

    // If any ID is free, use it
    game_entity* Entity = Insert(&State->Entities);
    Entity->Type = Type;
    Entity->Transform = Transform(Position, Rotation, S);
    Entity->Active = Active;
    Entity->Collider = Collider;
    Entity->Parent = NULL;
    strcpy_s(Entity->Name, Name);

    return Entity;
}

void RemoveEntity(game_entity_state* State, int EntityID) {
    game_entity* Entity = &State->Entities.List[EntityID];
    Assert(!Entity->Active);

    switch(Entity->Type) {
        case Entity_Type_Camera: {
            Remove(&State->Cameras, Entity->Index);
        } break;

        case Entity_Type_Character: {
            Remove(&State->Characters, Entity->Index);
        } break;

        case Entity_Type_Enemy: {
            Remove(&State->Enemies, Entity->Index);
        } break;

        case Entity_Type_Prop: {
            Remove(&State->Props, Entity->Index);
        } break;

        case Entity_Type_Weapon: {
            Remove(&State->Weapons, Entity->Index);
        } break;

        default: Raise("Invalid entity type.");
    }

    *Entity = {};
    State->Entities.Count--;
    State->Entities.FreeIDs[State->Entities.nFreeIDs] = EntityID;
    State->Entities.nFreeIDs++;
}

game_entity* QueryEntity(game_entity_state* State, game_entity_type Type, bool Active = true) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        game_entity* Entity = &State->Entities.List[i];
        if (Entity->Type == Type && Entity->Active == Active) {
            return Entity;
        }
    }
    return 0;
}

int QueryEntityCount(game_entity_state* State, game_entity_type Type, bool Active = true) {
    int Result = 0;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        game_entity* Entity = &State->Entities.List[i];
        if (Entity->Type == Type && Entity->Active == Active) {
            Result++;
        }
    }
    return Result;
}

// Entity initialization ___________________________________________________________________________________________________________________

camera* AddCamera(
    game_entity_state* State,
    v3 Position,
    float Angle, float Pitch,
    float Distance = 9.0
) {
    Assert(State->Cameras.Count < MAX_CAMERAS);
    // If any ID is free, use it
    camera* Cam = Insert(&State->Cameras);
    Cam->Angle = Angle;
    Cam->Pitch = Pitch;
    Cam->Position = Position;
    Cam->Distance = Distance;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Camera %d", Cam->ID);

    quaternion Rotation = Quaternion(Cam->Angle * Degrees, V3(0,1,0)) * Quaternion(Cam->Pitch * Degrees, V3(1,0,0));
    game_entity* Entity = AddEntity(State, NameBuffer, Entity_Type_Camera, SphereCollider(Position, 1.0f), Position, Rotation, Scale(), Cam->ID == 0);
    Entity->Index = Cam->ID;
    Cam->Entity = Entity;

    return Cam;
}

character* AddCharacter(game_assets* Assets, game_entity_state* State, v3 Position, int MaxHP) {
    Assert(State->Characters.Count < MAX_CHARACTERS);
    // If any ID is free, use it
    int CharacterID = -1;
    if (State->Characters.nFreeIDs > 0) {
        CharacterID = State->Characters.FreeIDs[State->Characters.nFreeIDs - 1];
        State->Characters.FreeIDs[State->Characters.nFreeIDs-- - 1] = -1;
        State->Characters.Count++;
    }
    else CharacterID = State->Characters.Count++;

    character* pCharacter = &State->Characters.List[CharacterID];
    pCharacter->Animator.Active = false;
    pCharacter->Animator.Animation = GetAsset(Assets, Animation_Walk_ID);
    game_mesh* Mesh = GetAsset(Assets, Mesh_Body_ID);
    pCharacter->Armature = Mesh->Armature;
    pCharacter->Animator.Armature = &pCharacter->Armature;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Character %d", CharacterID);

    quaternion Rotation = Quaternion(1.5f * Pi, V3(0,1,0));
    pCharacter->Entity = AddEntity(
        State, 
        NameBuffer, 
        Entity_Type_Character,
        CapsuleCollider(V3(0,0.6f,0), V3(0,3.0f,0), 0.8f),
        Position, 
        Rotation, 
        Scale()
    );
    pCharacter->Entity->Index = CharacterID;

    return pCharacter;
}

enemy* AddEnemy(game_entity_state* State, v3 Position) {
    Assert(State->Characters.Count < MAX_ENEMIES);
    // If any ID is free, use it
    int EnemyID = -1;
    if (State->Enemies.nFreeIDs > 0) {
        EnemyID = State->Enemies.FreeIDs[State->Enemies.nFreeIDs - 1];
        State->Enemies.FreeIDs[State->Enemies.nFreeIDs-- - 1] = -1;
        State->Enemies.Count++;
    }
    else EnemyID = State->Enemies.Count++;

    enemy* pEnemy = &State->Enemies.List[EnemyID];
    char NameBuffer[32];
    sprintf_s(NameBuffer, "Enemy %d", EnemyID);

    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
    pEnemy->Entity = AddEntity(State, NameBuffer, Entity_Type_Enemy, SphereCollider(V3(0,0,0), 1.5f), Position, Rotation, Scale());
    pEnemy->Entity->Index = EnemyID;
    return pEnemy;
}

prop* AddProp(
    game_entity_state* State, 
    game_mesh_id MeshID,
    color Color = White,
    v3 Position = V3(0,0,0),
    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0),
    scale S = Scale()
) {
    Assert(State->Props.Count < MAX_PROPS);
    // If any ID is free, use it
    int PropID = -1;
    if (State->Props.nFreeIDs > 0) {
        PropID = State->Props.FreeIDs[State->Props.nFreeIDs - 1];
        State->Props.FreeIDs[State->Props.nFreeIDs-- - 1] = -1;
        State->Props.Count++;
    }
    else PropID = State->Props.Count++;

    prop* pProp = &State->Props.List[PropID];
    pProp->MeshID = MeshID;
    pProp->Color = Color;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Prop %d", PropID);

    pProp->Entity = AddEntity(State, NameBuffer, Entity_Type_Prop, SphereCollider(V3(0,0,0), 5.0f), Position, Rotation, S);
    pProp->Entity->Index = PropID;
    return pProp;
}

weapon* AddWeapon(   
    game_entity_state* State,
    weapon_type Type,
    color Color = White,
    v3 Position = V3(0,0,0),
    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0),
    scale S = Scale()
) {
    Assert(State->Weapons.Count < MAX_PROPS);
    // If any ID is free, use it
    weapon* pWeapon = Insert(&State->Weapons);
    pWeapon->Type = Type;
    pWeapon->ParentBone = -1;
    pWeapon->Color = Color;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Weapon %d", pWeapon->ID);

    collider Collider;
    switch (pWeapon->Type) {
        case Weapon_Sword: Collider = CapsuleCollider(V3(0,0,0), V3(0,3,0), 0.5f); break;
        case Weapon_Shield: Collider = CapsuleCollider(V3(0,-0.3,0), V3(0,0.7,0), 1.0f); break;
        default: Assert(false);
    }

    pWeapon->Entity = AddEntity(
        State, 
        NameBuffer, 
        Entity_Type_Weapon,
        Collider,
        Position, 
        Rotation, 
        S
    );
    pWeapon->Entity->Index = pWeapon->ID;
    return pWeapon;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Game state                                                                                                                                   |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct game_state {
    game_entity_state Entities;
    particle_emitter* Emitter;
    camera* ActiveCamera;
    character* ControlledCharacter;
    double dt;
    float Time;
    bool Exit;
};

void UpdateGameState(game_assets* Assets, game_state* State, game_input* Input, float Width, float Height) {
    game_entity_state* EntityState = &State->Entities;
    uint32 Index = 0;

// Cameras _________________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nCameras = EntityState->Cameras.Count;
    while(nCameras > 0) {
        camera* Cam = &EntityState->Cameras.List[Index++];
        game_entity* Entity = (game_entity*)Cam->Entity;

        if (Cam->OnAir) {
            State->ActiveCamera = Cam;
        }

    // Zoom
        if (Input->Mode == Keyboard) {
            if (Input->Mouse.Wheel > 0)      Cam->Distance /= 1.2;
            else if (Input->Mouse.Wheel < 0) Cam->Distance *= 1.2;
        }

    // Orbit around position
        if (
            Input->Mode == Keyboard && 
            Input->Mouse.MiddleClick.IsDown && 
            Input->Mouse.MiddleClick.WasDown &&
            Input->Mouse.Cursor.X >= 0 && Input->Mouse.Cursor.X <= Width &&
            Input->Mouse.Cursor.Y >= 0 && Input->Mouse.Cursor.Y <= Height
        ) {
            v2 Offset = Input->Mouse.Cursor - Input->Mouse.LastCursor;
            double AngularVelocity = 0.5;

            Cam->Angle -= AngularVelocity * Offset.X;
            Cam->Pitch += AngularVelocity * Offset.Y;
        }

        if (Input->Mode == Controller) {
            v2 Joystic = V2(Input->Controller.RightJoystick.X, Input->Controller.RightJoystick.Y);

            if (modulus(Joystic) > 0.1) {
                Cam->Angle -= 3.0 * Joystic.X;
                Cam->Pitch -= 3.0 * Joystic.Y;
            }
        }

    // Camera basis
        Cam->Basis = GetCameraBasis(Cam->Angle, Cam->Pitch);
        Cam->View = GetViewMatrix(Cam);
    
    // Movement
        v3 Direction = V3(0,0,0);
        float Speed = 20.0f;
        if (Input->Mode == Keyboard) {
            bool Left = Input->Keyboard.A.IsDown;
            bool Right = Input->Keyboard.D.IsDown;
            bool Up = Input->Keyboard.W.IsDown;
            bool Down = Input->Keyboard.S.IsDown;
            if (Right) { Direction.X += 1.0f; }
            if (Left)  { Direction.X -= 1.0f; }
            if (Up)    { Direction.Z += 1.0f; }
            if (Down)  { Direction.Z -= 1.0f; }
            Direction = normalize(Direction);
            if (Input->Keyboard.Space.IsDown) Direction.Y += 1.0f;
            if (Input->Keyboard.Shift.IsDown) Direction.Y -= 1.0f;
        }
        else if (Input->Mode == Controller) {
            v2 Normalized = normalize(Input->Controller.LeftJoystick);
            Direction.X = Normalized.X;
            Direction.Z = Normalized.Y;
            Speed = 20.0f * modulus(Input->Controller.LeftJoystick);
        }

        basis HorizontalBasis = GetCameraBasis(Cam->Angle, 0);
        // Direction is in coordinates relative to camera
        float Angle = atan2f(-Direction.X, Direction.Z);
        Direction = Direction.Y * V3(0.0, 1.0, 0.0) + Direction.X * HorizontalBasis.X - Direction.Z * HorizontalBasis.Z;
        Cam->Entity->Velocity = Speed * Direction;
        Cam->Entity->Transform.Rotation = Quaternion(State->ActiveCamera->Angle * Degrees + Angle, V3(0,1,0));

        Cam->Position += State->dt * Cam->Entity->Velocity;

        break;
    }
    
// Characters ______________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nCharacters = EntityState->Characters.Count;
    for (int i = 0; i < EntityState->Characters.Count; i++) {
        character* Character = &EntityState->Characters.List[i];

        if (Character->Armature.nBones == 0) {
            Character->Armature = GetAsset(Assets, Mesh_Body_ID)->Armature;
            Character->Animator.Armature = &Character->Armature;
            Character->Animator.Animation = GetAsset(Assets, Animation_Idle_ID);
            Character->Animator.Loop = true;
            Character->Animator.Active = true;
            Character->Action.ID = Character_Action_Idle_ID;
            Character->Action.Loop = true;
        }
        
        Character->Entity->Collided = false;

        Update(&Character->Animator);
    }
    
// Movement _______________________________________________________________________________________________________________________
    if (State->ControlledCharacter != NULL && State->ControlledCharacter->Entity != NULL) {
        character* Character = State->ControlledCharacter;

        // Actions
        character_action_id PastAction = Character->Action.ID;
        Character->Action = GetCharacterAction(Character, Input);
        character_action_id NewAction = Character->Action.ID;
        Character->Animator.Animation = GetAsset(Assets, Character->Action.AnimationID);

        if (PastAction == Character_Action_Jump_ID) {
            Character->Entity->Collider.Capsule.Segment.Head += V3(0,Character->Armature.Bones[0].Transform.Translation.Y,0);
            Character->Entity->Collider.Capsule.Segment.Tail += V3(0,Character->Armature.Bones[0].Transform.Translation.Y,0);
        }

        if (PastAction == Character_Action_Attack_ID && NewAction == Character_Action_Idle_ID) {
            Character->Entity->Transform.Translation += Character->Entity->Transform.Rotation * V3(0,0,2);
        }

        Character->Entity->Velocity = V3(0, 0, 0);

        if (Character->Action.ID == Character_Action_Walk_ID || Character->Action.ID == Character_Action_Jump_ID) {
            v3 Direction = V3(0,0,0);
            float Speed = 20.0f;
            if (Input->Mode == Keyboard) {
                bool Left = Input->Keyboard.A.IsDown;
                bool Right = Input->Keyboard.D.IsDown;
                bool Up = Input->Keyboard.W.IsDown;
                bool Down = Input->Keyboard.S.IsDown;
                if (Right) { Direction.X += 1.0; }
                if (Left)  { Direction.X -= 1.0; }
                if (Up) { Direction.Z += 1.0; }
                if (Down) { Direction.Z -= 1.0; }
                Direction = normalize(Direction);
            }
            else if (Input->Mode == Controller) {
                v2 Normalized = normalize(Input->Controller.LeftJoystick);
                Direction.X = Normalized.X;
                Direction.Z = Normalized.Y;
                Speed = 20.0f * modulus(Input->Controller.LeftJoystick);
            }

            basis HorizontalBasis = GetCameraBasis(State->ActiveCamera->Angle, 0);
            
            // Direction is in coordinates relative to camera
            float Angle = atan2f(-Direction.X, Direction.Z);
            Direction = Direction.Y * V3(0.0, 1.0, 0.0) + Direction.X * HorizontalBasis.X - Direction.Z * HorizontalBasis.Z;
            Character->Entity->Velocity = Speed * Direction;
            Character->Entity->Transform.Rotation = Quaternion(State->ActiveCamera->Angle * Degrees + Angle, V3(0,1,0));
        }
        Character->Entity->Transform.Translation += State->dt * Character->Entity->Velocity;
    }

// Enemies _________________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nEnemies = EntityState->Enemies.Count;
    while (nEnemies > 0) {
        enemy* pEnemy = &EntityState->Enemies.List[Index++];
        if (pEnemy->Entity != NULL) nEnemies--;
        else continue;

        pEnemy->Entity->Transform.Translation.Y = 3.2 + sin(3 * State->Time);

        v3 FacingDirection = V3(-1,0,0);
        if (State->ControlledCharacter != NULL && State->ControlledCharacter->Entity != NULL) {
            FacingDirection = State->ControlledCharacter->Entity->Transform.Translation - pEnemy->Entity->Transform.Translation;
        }
        float Angle = atan2f(FacingDirection.Z, FacingDirection.X);
        pEnemy->Entity->Transform.Rotation = Quaternion(Angle, V3(0,1,0));
    }

// Weapons _________________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nWeapons = EntityState->Enemies.Count;
    while (nEnemies > 0) {
        weapon* pWeapon = &EntityState->Weapons.List[Index++];
        if (pWeapon->ParentBone == -1) {
            pWeapon->Entity->Transform.Rotation = Quaternion(State->Time, V3(0,1,0));
        }

        pWeapon->Entity->Collided = false;

        if (State->ControlledCharacter != NULL && State->ControlledCharacter->Entity != NULL) {
            bool Collision = Collide(pWeapon->Entity, State->ControlledCharacter->Entity);
            if (pWeapon->Entity->Parent == NULL && Collision) {
                State->ControlledCharacter->Entity->Collided = true;
                pWeapon->Entity->Collided = true;
                Equip(pWeapon, State->ControlledCharacter);
            }
    
            if (pWeapon->ParentBone > 0) {
                bone Bone = State->ControlledCharacter->Armature.Bones[pWeapon->ParentBone];
                transform Transform = WeaponTransforms[pWeapon->Type];
                pWeapon->Entity->Transform = Transform * Bone.Transform * State->ControlledCharacter->Entity->Transform;
            }
        }
    }
}

void PushEntities(render_group* Group, camera* Camera, game_state* GameState, game_input* Input, float Time) {
    game_entity_state* State = &GameState->Entities;
    game_assets* Assets = Group->Assets;

    basis Basis = Camera->Basis;
    ray Ray = MouseRay(Group->Width, Group->Height, Camera->Position + Camera->Distance * Basis.Z, Basis, Input->Mouse.Cursor);
    int i = 0;
    int nEntities = State->Entities.Count;
    while (nEntities > 0 && i < MAX_ENTITIES) {
        game_entity* Entity = &State->Entities.List[i++];

        if (Entity->Active) nEntities--;
        else continue;

        collider Collider = Entity->Transform * Entity->Collider;
        Entity->Hovered = Raycast(Ray, Collider);
        bool Outline = Entity->Hovered;
        switch(Entity->Type) {
            case Entity_Type_Character: {
                character* pCharacter = &State->Characters.List[Entity->Index];
                game_mesh* Mesh = GetAsset(Assets, Mesh_Body_ID);
                PushMesh(
                    Group,
                    Mesh_Body_ID,
                    .Armature = &pCharacter->Armature,
                    .Transform = Entity->Transform,
                    .Outline = Outline
                );
            } break;
    
            case Entity_Type_Enemy: {
                enemy* pEnemy = &State->Enemies.List[Entity->Index];
                PushMesh(
                    Group,
                    Mesh_Enemy_ID,
                    .TextureID = Bitmap_Enemy_ID,
                    .Transform = Entity->Transform,
                    .Outline = Outline
                );
            } break;

            case Entity_Type_Prop: {
                prop* pProp = &State->Props.List[Entity->Index];
                PushMesh(
                    Group,
                    pProp->MeshID,
                    .Color = pProp->Color,
                    .Transform = Entity->Transform
                );
            } break;

            case Entity_Type_Weapon: {
                weapon* pWeapon = &State->Weapons.List[Entity->Index];
                game_mesh_id MeshID = pWeapon->Type == Weapon_Sword ? Mesh_Sword_ID : Mesh_Shield_ID;

                PushMesh(Group, MeshID, .Transform = Entity->Transform);
            } break;
        }

        if (Group->Debug && Group->DebugColliders && Entity->Type != Entity_Type_Camera) {
            PushCollider(Group, Entity->Collider, Entity->Transform, Entity->Collided ? Red : Yellow);
        }
    }
}

#endif