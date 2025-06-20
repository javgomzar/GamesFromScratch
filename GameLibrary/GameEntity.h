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
    Entity_Type_Character,
    Entity_Type_Enemy,
    Entity_Type_Camera,
    Entity_Type_Prop,
    Entity_Type_Weapon,

    game_entity_type_count
};

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

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Stats                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct stats {
    uint32 HP;
    uint32 MaxHP;
    uint32 Strength;
    uint32 Defense;
    uint32 Intelligence;
    uint32 Wisdom;
    float Speed;
    float Precission;
};

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Enemies                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct enemy {
    stats Stats;
    game_entity* Entity;
};

const int MAX_ENEMIES = 32;
DefineEntityList(MAX_ENEMIES, enemy);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Weapons                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

enum weapon_type {
    Weapon_Sword,
    Weapon_Shield,

    weapon_type_count
};

struct weapon {
    game_entity* Entity;
    weapon_type Type;
    color Color;
    int ParentBone;
};

const int MAX_WEAPONS = 32;
DefineEntityList(MAX_WEAPONS, weapon);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Character                                                                                                                                    |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

enum character_action_id {
    Character_Action_Idle_ID,
    Character_Action_Walk_ID,
    Character_Action_Jump_ID,
    Character_Action_Attack_ID
};

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
    armature Armature;
    stats Stats;
    game_animator Animator;
    character_action Action;
    game_entity* Entity;
    weapon* LeftHand;
    weapon* RightHand;
    character_class Class;
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

const int MAX_CHARACTERS = 1;
DefineEntityList(MAX_CHARACTERS, character);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Props                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct prop {
    int EntityID;
    game_mesh_id MeshID;
    game_bitmap_id Texture;
    game_shader_pipeline_id Shader;
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
    weapon_list Weapons;
};

int AddEntity(
    game_entity_list* List,
    const char* Name,
    game_entity_type Type,
    collider Collider,
    v3 Position = V3(0,0,0),
    quaternion Rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f),
    scale S = Scale(),
    bool Active = true
) {
    Assert(List->nEntities < MAX_ENTITIES);

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
    Entity->Transform = Transform(Position, Rotation, S);
    Entity->Active = Active;
    Entity->Collider = Collider;
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

camera* AddCamera(
    game_entity_list* List,
    v3 Position,
    float Angle, float Pitch,
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
    int EntityID = AddEntity(List, NameBuffer, Entity_Type_Camera, SphereCollider(Position, 1.0f), Position, Rotation, Scale(), CameraID == 0);
    Cam->EntityID = EntityID;
    game_entity* CameraEntity = &List->Entities[EntityID];
    CameraEntity->Index = CameraID;

    return Cam;
}

character* AddCharacter(game_entity_list* List, v3 Position, int MaxHP) {
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
    pCharacter->Animator.Active = false;
    pCharacter->Animator.Animation = GetAsset(List->Assets, Animation_Walk_ID);
    game_mesh* Mesh = GetAsset(List->Assets, Mesh_Body_ID);
    pCharacter->Armature = Mesh->Armature;
    pCharacter->Animator.Armature = &pCharacter->Armature;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Character %d", CharacterID);

    quaternion Rotation = Quaternion(1.5f * Pi, V3(0,1,0));
    int EntityID = AddEntity(
        List, 
        NameBuffer, 
        Entity_Type_Character,
        CapsuleCollider(V3(0,0.6f,0), V3(0,3.0f,0), 0.8f),
        Position, 
        Rotation, 
        Scale()
    );

    pCharacter->Stats.MaxHP = 100;
    pCharacter->Stats.HP = pCharacter->Stats.MaxHP;
    pCharacter->Stats.Strength = 10;
    pCharacter->Stats.Defense = 10;
    pCharacter->Stats.Intelligence = 10;
    pCharacter->Stats.Wisdom = 10;
    pCharacter->Stats.Speed = 10;
    pCharacter->Stats.Precission = 10;

    game_entity* CharacterEntity = &List->Entities[EntityID];
    pCharacter->Entity = CharacterEntity;
    CharacterEntity->Index = CharacterID;

    return pCharacter;
}

enemy* AddEnemy(game_entity_list* List, v3 Position, int MaxHP) {
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
    pEnemy->Stats.MaxHP = 50;
    pEnemy->Stats.HP = pEnemy->Stats.MaxHP;
    pEnemy->Stats.Strength = 10;
    pEnemy->Stats.Defense = 10;
    pEnemy->Stats.Intelligence = 10;
    pEnemy->Stats.Wisdom = 10;
    pEnemy->Stats.Speed = 6;
    pEnemy->Stats.Precission = 10;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Enemy %d", EnemyID);

    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
    int EntityID = AddEntity(List, NameBuffer, Entity_Type_Enemy, SphereCollider(V3(0,0,0), 1.5f), Position, Rotation, Scale());
    game_entity* EnemyEntity = &List->Entities[EntityID];
    pEnemy->Entity = EnemyEntity;
    EnemyEntity->Index = EnemyID;

    return pEnemy;
}

prop* AddProp(
    game_entity_list* List, 
    game_mesh_id MeshID, 
    game_shader_pipeline_id Shader, 
    color Color = White,
    v3 Position = V3(0,0,0),
    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0),
    scale S = Scale()
) {
    Assert(List->Props.Size < MAX_PROPS);
    // If any ID is free, use it
    int PropID = -1;
    if (List->Props.nFreeIDs > 0) {
        PropID = List->Props.FreeIDs[List->Props.nFreeIDs - 1];
        List->FreeIDs[List->nFreeIDs-- - 1] = -1;
        List->Props.Size++;
    }
    else PropID = List->Props.Size++;

    prop* pProp = &List->Props.List[PropID];
    pProp->MeshID = MeshID;
    pProp->Shader = Shader;
    pProp->Color = Color;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Prop %d", PropID);

    int EntityID = AddEntity(List, NameBuffer, Entity_Type_Prop, SphereCollider(V3(0,0,0), 5.0f), Position, Rotation, S);
    pProp->EntityID = EntityID;
    game_entity* PropEntity = &List->Entities[EntityID];
    PropEntity->Index = PropID;

    return pProp;
}

weapon* AddWeapon(   
    game_entity_list* List,
    weapon_type Type,
    color Color = White,
    v3 Position = V3(0,0,0),
    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0),
    scale S = Scale()
) {
    Assert(List->Weapons.Size < MAX_PROPS);
    // If any ID is free, use it
    int WeaponID = -1;
    if (List->Weapons.nFreeIDs > 0) {
        WeaponID = List->Weapons.FreeIDs[List->Weapons.nFreeIDs - 1];
        List->FreeIDs[List->nFreeIDs-- - 1] = -1;
        List->Weapons.Size++;
    }
    else WeaponID = List->Weapons.Size++;

    weapon* pWeapon = &List->Weapons.List[WeaponID];
    pWeapon->Type = Type;
    pWeapon->ParentBone = -1;
    pWeapon->Color = Color;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Weapon %d", WeaponID);

    collider Collider;
    switch (pWeapon->Type) {
        case Weapon_Sword: Collider = CapsuleCollider(V3(0,0,0), V3(0,3,0), 0.5f); break;
        case Weapon_Shield: Collider = CapsuleCollider(V3(0,-0.3,0), V3(0,0.7,0), 1.0f); break;
        default: Assert(false);
    }

    int EntityID = AddEntity(
        List, 
        NameBuffer, 
        Entity_Type_Weapon,
        Collider,
        Position, 
        Rotation, 
        S
    );
    game_entity* WeaponEntity = &List->Entities[EntityID];
    pWeapon->Entity = WeaponEntity;
    WeaponEntity->Index = WeaponID;

    return pWeapon;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Combat                                                                                                                                       |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

enum combatant_type {
    Combatant_Type_Player,
    Combatant_Type_Enemy,
};

struct combatant {
    game_entity* Entity;
    stats Stats;
    uint32 Index;
    float ATB;
    combatant_type Type;
};

combatant Combatant(character Character) {
    combatant Result;
    Result.Stats = Character.Stats;
    Result.Entity = Character.Entity;
    Result.ATB = 100.0f;
    Result.Type = Combatant_Type_Player;
    return Result;
}

combatant Combatant(enemy Enemy) {
    combatant Result;
    Result.Stats = Enemy.Stats;
    Result.Entity = Enemy.Entity;
    Result.ATB = 100.0f;
    Result.Type = Combatant_Type_Enemy;
    return Result;
}

void ReceiveDamage(combatant* Combatant, int Damage) {
    if (Damage > Combatant->Stats.HP) {
        Combatant->Stats.HP = 0;
    }
    else Combatant->Stats.HP -= Damage;
}

const int MAX_COMBATANTS = 32;
struct turn {
    combatant* Attacker;
    uint32 Index;
    uint32 nTargets;
    combatant* Targets[MAX_COMBATANTS];
    float ATB[MAX_COMBATANTS];
    float ATBCost;
};

const int MAX_HISTORY_SIZE = 128;
ArrayDefinition(MAX_HISTORY_SIZE, turn);

ArrayDefinition(MAX_COMBATANTS, combatant);

const int TURN_BUFFER_SIZE = 16;
struct game_combat {
    turn_array History;
    combatant_array Combatants;
    turn NextTurns[TURN_BUFFER_SIZE];
    turn Turn;
    bool Active;
    
    // Advances ATB of turn. If a new attacker is found, it is returned; returns NULL otherwise.
    combatant* AdvanceTurnATB(turn& T) {
        combatant* Result = NULL;
        float MaxSpeed = 0.0f;
        for (int i = 0; i < Combatants.Count; i++) {
            combatant* Combatant = &Combatants.Content[i];
            if (Combatant->Stats.HP > 0) {
                T.ATB[i] += Combatant->Stats.Speed;
                if (T.ATB[i] >= 100.0f) {
                    if (
                        Combatant->Stats.Speed > MaxSpeed || 
                        // If current attacker's speed is equal to this potential attacker, flip a coin
                        Combatant->Stats.Speed == MaxSpeed && Bernoulli()
                    ) Result = Combatant;
                }
                T.ATB[i] = Clamp(T.ATB[i], 0.0f, 100.0f);
            }
        }
        return Result;
    }

    // Applies ATB cost and advances turn ATB until new attacker is found.
    turn GetNextTurn(turn PreviousTurn) {
        turn Result = PreviousTurn;
        Result.Index++;
        Result.Attacker = 0;
        Result.nTargets = 0;
        for (int i = 0; i < Combatants.Count; i++) {
            Result.Targets[i] = 0;
        }

        // Apply ATB Cost
        Result.ATB[PreviousTurn.Attacker->Index] -= PreviousTurn.ATBCost;

        while (Result.Attacker == 0) {
            Result.Attacker = AdvanceTurnATB(Result);
        }
        return Result;
    }

    void Erase() {
        Clear(&History);
        Clear(&Combatants);
        Turn = {};
        for (int i = 0; i < TURN_BUFFER_SIZE; i++) {
            NextTurns[i] = {};
        }
    }

    void Start(game_entity_list* List) {
        Erase();
        Active = true;

        // Add entities to struct and compute first attacker
        float MaxSpeed = 0.0f;
        for (int i = 0; i < List->nEntities; i++) {
            game_entity* Entity = &List->Entities[i];
            combatant EntityCombatant;
            bool IsEnemy = Entity->Type == Entity_Type_Enemy;
            bool IsCharacter = Entity->Type == Entity_Type_Character;
            if (IsEnemy || IsCharacter) {
                if (IsEnemy) {
                    enemy Enemy = List->Enemies.List[Entity->Index];
                    EntityCombatant = Combatant(Enemy);
                }
                else if (IsCharacter) {
                    character Character = List->Characters.List[Entity->Index];
                    EntityCombatant = Combatant(Character);
                }

                EntityCombatant.Index = Combatants.Count;
                combatant* Combatant = &Combatants.Content[EntityCombatant.Index];
                Append(&Combatants, EntityCombatant);
                Turn.ATB[Combatant->Index] = Combatant->ATB;

                if (EntityCombatant.Stats.Speed > MaxSpeed) {
                    Turn.Attacker = Combatant;
                    MaxSpeed = EntityCombatant.Stats.Speed;
                }
                else if (Combatant->Stats.Speed == MaxSpeed) {
                    float Random = (float)rand() / (float)RAND_MAX;
                    if (Random >= 0.5f) {
                        Turn.Attacker = Combatant;
                        MaxSpeed = EntityCombatant.Stats.Speed;
                    }
                }
            }
        }

        Turn.Index = 0;
        Turn.ATBCost = 50.0f;
        Turn.nTargets = 0;

        // Fill turn buffer
        NextTurns[0] = GetNextTurn(Turn);
        for (int i = 1; i < TURN_BUFFER_SIZE; i++) {
            NextTurns[i] = GetNextTurn(NextTurns[i-1]);
        }
    }

    void EndTurn() {
        Append(&History, Turn);
        Turn = NextTurns[0];
        for (int i = 1; i < TURN_BUFFER_SIZE; i++) {
            NextTurns[i-1] = NextTurns[i];
        }
        turn LastKnown = NextTurns[TURN_BUFFER_SIZE - 1];
        NextTurns[TURN_BUFFER_SIZE - 1] = GetNextTurn(LastKnown);
    }

    void End() {
        Erase();
        Active = false;
    }
};

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Game state                                                                                                                                   |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct game_state {
    game_entity_list EntityList;
    game_combat Combat;
    double dt;
    float Time;
    bool Exit;
};

void Update(camera** pActiveCamera, game_state* State, game_input* Input, float Width, float Height) {
    TIMED_BLOCK;
    game_entity_list* List = &State->EntityList;
    game_combat* Combat = &State->Combat; 

// Combat
    if (!State->Combat.Active && Input->Keyboard.One.JustPressed) {
        Combat->Start(List);
    }

    if (State->Combat.Active) {
        bool End = true;
        for (int i = 0; i < State->Combat.Combatants.Count; i++) {
            if (State->Combat.Combatants.Content[i].Type == Combatant_Type_Enemy) {
                End = false;
                break;
            }
        }
        if (End) Combat->End();
    }

// Cameras _________________________________________________________________________________________________________________________________
    camera* ActiveCamera = 0;

    for (int i = 0; i < List->Cameras.Size; i++) {
        camera* Cam = &List->Cameras.List[i];
        game_entity* CamEntity = &List->Entities[Cam->EntityID];
        if (CamEntity->Active) {
            *pActiveCamera = Cam;
            ActiveCamera = Cam;

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

        // Rotation
            Cam->Basis = GetCameraBasis(Cam->Angle, Cam->Pitch);
            quaternion Rotation = Quaternion(Cam->Angle * Degrees, V3(0,1,0)) * Quaternion(Cam->Pitch * Degrees, V3(-1,0,0));
            transform Test = Transform(Rotation);
            matrix4 MatrixT = Matrix(Test);

            break;
        }
    }
        
// Characters ______________________________________________________________________________________________________________________________
    character* ControlledCharacter;
    game_entity* ControlledCharacterEntity;
    for (int i = 0; i < List->Characters.Size; i++) {
        character* Character = &List->Characters.List[i];
        game_entity* CharacterEntity = Character->Entity;
        
        CharacterEntity->Velocity = V3(0, 0, 0);

        character_action_id PastAction = Character->Action.ID;
        Character->Action = GetCharacterAction(Character, Input);
        character_action_id NewAction = Character->Action.ID;
        Character->Animator.Animation = GetAsset(List->Assets, Character->Action.AnimationID);

        if (PastAction == Character_Action_Jump_ID) {
            CharacterEntity->Collider.Capsule.Segment.Head += V3(0,Character->Armature.Bones[0].Transform.Translation.Y,0);
            CharacterEntity->Collider.Capsule.Segment.Tail += V3(0,Character->Armature.Bones[0].Transform.Translation.Y,0);
        }

        // Movement
        if (NewAction == Character_Action_Walk_ID || NewAction == Character_Action_Jump_ID) {
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

            basis HorizontalBasis = GetCameraBasis(ActiveCamera->Angle, 0);
            
            // Direction is in coordinates relative to camera
            float Angle = atan2f(-Direction.X, Direction.Z);
            Direction = Direction.Y * V3(0.0, 1.0, 0.0) + Direction.X * HorizontalBasis.X - Direction.Z * HorizontalBasis.Z;
            CharacterEntity->Velocity = Speed * Direction;
            CharacterEntity->Transform.Rotation = Quaternion(ActiveCamera->Angle * Degrees + Angle, V3(0,1,0));
        }
        CharacterEntity->Transform.Translation += State->dt * CharacterEntity->Velocity;

        if (PastAction == Character_Action_Attack_ID && NewAction == Character_Action_Idle_ID) {
            CharacterEntity->Transform.Translation += CharacterEntity->Transform.Rotation * V3(0,0,2);
        }
        
        CharacterEntity->Collided = false;
        CharacterEntity->Collider.Capsule.Segment = { V3(0,0.75f,0), V3(0,3.75f,0) };

        ControlledCharacter = Character;
        ControlledCharacterEntity = CharacterEntity;

        Update(&Character->Animator);
    }
    
// Autofollow player _______________________________________________________________________________________________________________________
    v3 Displacement = ControlledCharacterEntity->Transform.Translation - ActiveCamera->Position;
    Displacement.Y = 0;
    float Distance = modulus(Displacement);
    v3 Velocity = V3(0,0,0);
    float MinDistance = .01f;
    if (Distance >= MinDistance) Velocity = 20.0f * (Distance - MinDistance) * normalize(Displacement);
    ActiveCamera->Position += State->dt * Velocity;
    game_entity* ActiveCameraEntity = &List->Entities[ActiveCamera->EntityID];
    ActiveCameraEntity->Transform.Translation = V3(0,0,ActiveCamera->Distance) - ActiveCamera->Position * ActiveCamera->Basis;

// Enemies _________________________________________________________________________________________________________________________________
    for (int i = 0; i < List->Enemies.Size; i++) {
        enemy* pEnemy = &List->Enemies.List[i];
        pEnemy->Entity->Transform.Translation.Y = 3.2 + sin(3 * State->Time);
        v3 FacingDirection = ControlledCharacterEntity->Transform.Translation - pEnemy->Entity->Transform.Translation;
        float Angle = atan2f(FacingDirection.Z, FacingDirection.X);
        pEnemy->Entity->Transform.Rotation = Quaternion(Angle, V3(0,1,0));
    }

// Weapons _________________________________________________________________________________________________________________________________
    for (int i = 0; i < List->Weapons.Size; i++) {
        weapon* pWeapon = &List->Weapons.List[i];
        game_entity* WeaponEntity = pWeapon->Entity;
        if (pWeapon->ParentBone == -1) {
            WeaponEntity->Transform.Rotation = Quaternion(State->Time, V3(0,1,0));
        }

        WeaponEntity->Collided = false;

        bool Collision = Collide(WeaponEntity, ControlledCharacterEntity);
        if (!WeaponEntity->Parent && Collision) {
            ControlledCharacterEntity->Collided = true;
            WeaponEntity->Collided = true;
            Equip(pWeapon, ControlledCharacter);
        }

        if (pWeapon->ParentBone > 0) {
            bone Bone = ControlledCharacter->Armature.Bones[pWeapon->ParentBone];
            transform ModelTransform;
            if (pWeapon->Type == Weapon_Sword) {
                ModelTransform = Transform(
                    V3(0.5f,2.0f,0),
                    Quaternion(-0.25f * Tau, V3(0,1,0)) * Quaternion(-0.25f * Tau, V3(1,0,0)),
                    WeaponEntity->Transform.Scale
                );
            }
            else if (pWeapon->Type == Weapon_Shield) {
                ModelTransform = Transform(
                    V3(-0.7f,2.2f,0),
                    Quaternion(0.5f * Tau, V3(0,0,1)) * Quaternion(0.25f * Tau, V3(1,0,0)),
                    WeaponEntity->Transform.Scale
                );
            }
            WeaponEntity->Transform = ModelTransform * Bone.Transform * ControlledCharacterEntity->Transform;
        }
    }
}

#endif