#ifndef GAME_ENTITY
#define GAME_ENTITY

#include "GameMath.h"
#include "GamePlatform.h"
#include "GameAssets.h"
#include "GameInput.h"
#include "GameRender.h"

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Entities                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

INTROSPECT
enum game_entity_type {
    Entity_Type_Character,
    Entity_Type_Enemy,
    Entity_Type_Camera,
    Entity_Type_Prop,
    Entity_Type_Weapon,

    game_entity_type_count
};

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

#define DefineEntityListRemove(type) void Remove(type##_list* List, int Index) { Assert(List->Count > 0); List->Count--; List->List[Index] = {}; List->FreeIDs[List->nFreeIDs++] = Index;}
#define DefineEntityList(maxNumber, type) struct type##_list {int nFreeIDs; int FreeIDs[maxNumber]; int Count; type List[maxNumber];}; DefineEntityListRemove(type)

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Camera                                                                                                                                       |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_CAMERAS = 16;
DefineEntityList(MAX_CAMERAS, camera);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Stats                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

INTROSPECT
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

stats Stats(uint32 MaxHP, uint32 Strength, uint32 Defense, uint32 Intelligence, uint32 Wisdom, float Speed, float Precission) {
    return {
        MaxHP, MaxHP, Strength, Defense, Intelligence, Wisdom, Speed, Precission
    };
};

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Enemies                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

INTROSPECT
enum enemy_type {
    Enemy_Type_Horns,
    Enemy_Type_Dog,
    Enemy_Type_Dyno,

    enemy_type_count
};

inline enemy_type RandomEnemyType() {
    return (enemy_type)RandInt(0, enemy_type_count);
}

INTROSPECT
struct enemy {
    game_entity* Entity;
    stats Stats;
    enemy_type Type;
    game_mesh_id MeshID;
    game_bitmap_id TextureID;
};

enemy EnemyTemplates[enemy_type_count] = {
    {
        NULL,
        Stats(50, 7, 10, 5, 5, 6, 10),
        Enemy_Type_Horns,
        Mesh_Horns_ID,
        Bitmap_Enemy_ID,
    },
    {
        NULL,
        Stats(20, 2, 8, 2, 6, 10, 10),
        Enemy_Type_Dog,
        Mesh_Dog_ID,
        Bitmap_Empty_ID,
    },
    {
        NULL,
        Stats(100, 10, 8, 1, 3, 5, 10),
        Enemy_Type_Dyno,
        Mesh_Dyno_ID,
        Bitmap_Empty_ID,
    },
};

v3 EnemyColliderOffset[enemy_type_count] = {
    V3(0,0,0),
    V3(0,1,0),
    V3(0,2,0)
};

char* EnemyNames[enemy_type_count] = {
    "Horns",
    "Dog",
    "Dyno"
};

enemy NewEnemy(enemy_type Type) {
    return EnemyTemplates[Type];
}

const int MAX_ENEMIES = 32;
DefineEntityList(MAX_ENEMIES, enemy);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Weapons                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

INTROSPECT
enum weapon_type {
    Weapon_Sword,
    Weapon_Shield,

    weapon_type_count
};

INTROSPECT
struct weapon {
    weapon_type Type;
    color Color;
    game_entity* Entity;
    int ParentBone;
};

const int MAX_WEAPONS = 32;
DefineEntityList(MAX_WEAPONS, weapon);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Character                                                                                                                                    |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

INTROSPECT
enum character_action_id {
    Character_Action_Idle_ID,
    Character_Action_Walk_ID,
    Character_Action_Jump_ID,
    Character_Action_Attack_ID
};

INTROSPECT
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

INTROSPECT
struct character {
    armature Armature;
    stats Stats;
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
DefineEntityList(MAX_CHARACTERS, character);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Props                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct prop {
    game_mesh_id MeshID;
    game_bitmap_id Texture;
    game_shader_pipeline_id Shader;
    color Color;
    game_entity* Entity;
};

const int MAX_PROPS = 32;
DefineEntityList(MAX_PROPS, prop);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Entity List                                                                                                                                  |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const int32 MAX_ENTITIES = MAX_CAMERAS + MAX_CHARACTERS + MAX_ENEMIES + MAX_PROPS;
DefineEntityList(MAX_ENTITIES, game_entity);

struct game_entity_state {
    game_entity_list Entities;
    camera_list Cameras;
    character_list Characters;
    enemy_list Enemies;
    prop_list Props;
    weapon_list Weapons;
    character* ControlledCharacter;
    camera* ActiveCamera;
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
    int EntityID = -1;
    if (State->Entities.nFreeIDs > 0) {
        EntityID = State->Entities.FreeIDs[State->Entities.nFreeIDs - 1];
        State->Entities.FreeIDs[State->Entities.nFreeIDs-- - 1] = -1;
        State->Entities.Count++;
    }
    else {
        EntityID = State->Entities.Count++;
    }

    game_entity* Entity = &State->Entities.List[EntityID];
    Entity->ID = EntityID;
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
    int CameraID = -1;
    if (State->Cameras.nFreeIDs > 0) {
        CameraID = State->Cameras.FreeIDs[State->Cameras.nFreeIDs - 1];
        State->Cameras.FreeIDs[State->Cameras.nFreeIDs-- - 1] = -1;
        State->Cameras.Count++;
    }
    else CameraID = State->Cameras.Count++;

    camera* Cam = &State->Cameras.List[CameraID];
    Cam->Angle = Angle;
    Cam->Pitch = Pitch;
    Cam->Position = Position;
    Cam->Distance = Distance;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Camera %d", CameraID);

    quaternion Rotation = Quaternion(Cam->Angle * Degrees, V3(0,1,0)) * Quaternion(Cam->Pitch * Degrees, V3(1,0,0));
    game_entity* Entity = AddEntity(State, NameBuffer, Entity_Type_Camera, SphereCollider(Position, 1.0f), Position, Rotation, Scale(), CameraID == 0);
    Entity->Index = CameraID;
    Cam->Entity = (void*)Entity;

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

    pCharacter->Stats.MaxHP = 100;
    pCharacter->Stats.HP = pCharacter->Stats.MaxHP;
    pCharacter->Stats.Strength = 10;
    pCharacter->Stats.Defense = 10;
    pCharacter->Stats.Intelligence = 10;
    pCharacter->Stats.Wisdom = 10;
    pCharacter->Stats.Speed = 10;
    pCharacter->Stats.Precission = 10;

    return pCharacter;
}

enemy* AddEnemy(game_entity_state* State, v3 Position, enemy_type Type) {
    Assert(State->Characters.Count < MAX_ENEMIES);
    static int32 EnemyQuantities[enemy_type_count] = {};

    // If any ID is free, use it
    int EnemyID = -1;
    if (State->Enemies.nFreeIDs > 0) {
        EnemyID = State->Enemies.FreeIDs[State->Enemies.nFreeIDs - 1];
        State->Enemies.FreeIDs[State->Enemies.nFreeIDs-- - 1] = -1;
        State->Enemies.Count++;
    }
    else EnemyID = State->Enemies.Count++;

    enemy* pEnemy = &State->Enemies.List[EnemyID];
    *pEnemy = NewEnemy(Type);

    char NameBuffer[32];
    sprintf_s(NameBuffer, "%s %d", EnemyNames[Type], EnemyQuantities[Type]++);

    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
    v3 ColliderOffset = EnemyColliderOffset[Type];
    pEnemy->Entity = AddEntity(State, NameBuffer, Entity_Type_Enemy, SphereCollider(ColliderOffset, 1.5f), Position, Rotation, Scale());
    pEnemy->Entity->Index = EnemyID;
    return pEnemy;
}

prop* AddProp(
    game_entity_state* State, 
    game_mesh_id MeshID, 
    game_shader_pipeline_id Shader, 
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
    pProp->Shader = Shader;
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
    int WeaponID = -1;
    if (State->Weapons.nFreeIDs > 0) {
        WeaponID = State->Weapons.FreeIDs[State->Weapons.nFreeIDs - 1];
        State->Weapons.FreeIDs[State->Weapons.nFreeIDs-- - 1] = -1;
        State->Weapons.Count++;
    }
    else WeaponID = State->Weapons.Count++;

    weapon* pWeapon = &State->Weapons.List[WeaponID];
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

    pWeapon->Entity = AddEntity(
        State, 
        NameBuffer, 
        Entity_Type_Weapon,
        Collider,
        Position, 
        Rotation, 
        S
    );
    pWeapon->Entity->Index = WeaponID;
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
    stats* Stats;
    uint32 Index;
    float ATB;
    combatant_type Type;
};

combatant Combatant(character* Character) {
    combatant Result;
    Result.Stats = &Character->Stats;
    Result.Entity = Character->Entity;
    Result.ATB = 100.0f;
    Result.Type = Combatant_Type_Player;
    return Result;
}

combatant Combatant(enemy* Enemy) {
    combatant Result;
    Result.Stats = &Enemy->Stats;
    Result.Entity = Enemy->Entity;
    Result.ATB = 100.0f;
    Result.Type = Combatant_Type_Enemy;
    return Result;
}

bool IsAlive(combatant* Combatant) {
    return Combatant->Stats->HP > 0;
}

void ReceiveDamage(combatant* Combatant, int Damage) {
    if (Damage > Combatant->Stats->HP) {
        Combatant->Stats->HP = 0;
    }
    else Combatant->Stats->HP -= Damage;
}

enum combatant_action {
    combatant_empty_action,
    combatant_attack_action,
    combatant_magic_action,
    combatant_items_action,
    combatant_flee_action,
};

const int MAX_COMBATANTS = 32;
struct turn {
    combatant* Attacker;
    uint32 Index;
    uint32 nTargets;
    combatant* Targets[MAX_COMBATANTS];
    float ATB[MAX_COMBATANTS];
    float ATBCost;
    combatant_action Action;
    bool TargetsSelected;
};

ArrayDefinition(MAX_COMBATANTS, combatant);

const int TURN_BUFFER_SIZE = 16;
struct game_combat {
    combatant_array Combatants;
    turn NextTurns[TURN_BUFFER_SIZE];
    turn Turn;
    memory_arena TurnsArena;
    game_entity_state* State;
    bool Active;
    
    // Advances ATB of turn. If a new attacker is found, it is returned; returns NULL otherwise.
    combatant* AdvanceTurnATB(turn& T, int AttackerIndex = -1) {
        combatant* Result = NULL;
        float MaxSpeed = 0.0f;
        for (int i = 0; i < Combatants.Count; i++) {
            combatant* Combatant = &Combatants.Content[i];
            if (IsAlive(Combatant)) {
                if (AttackerIndex != i) T.ATB[i] += Combatant->Stats->Speed;
                if (T.ATB[i] >= 100.0f) {
                    if (
                        Combatant->Stats->Speed > MaxSpeed || 
                        // If current attacker's speed is equal to this potential attacker, flip a coin
                        Combatant->Stats->Speed == MaxSpeed && Bernoulli()
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
        Result.Action = combatant_empty_action;
        for (int i = 0; i < Combatants.Count; i++) {
            Result.Targets[i] = 0;
        }

        // Apply ATB Cost
        Result.ATB[PreviousTurn.Attacker->Index] -= PreviousTurn.ATBCost;
        Result.Attacker = AdvanceTurnATB(Result, PreviousTurn.Attacker->Index);

        while (Result.Attacker == 0) {
            Result.Attacker = AdvanceTurnATB(Result);
        }
        return Result;
    }

    void Erase() {
        ClearArena(&TurnsArena);
        Clear(&Combatants);
        Turn = {};
        for (int i = 0; i < TURN_BUFFER_SIZE; i++) {
            NextTurns[i] = {};
        }
    }

    void FillTurnBuffer() {
        NextTurns[0] = GetNextTurn(Turn);
        for (int i = 1; i < TURN_BUFFER_SIZE; i++) {
            NextTurns[i] = GetNextTurn(NextTurns[i-1]);
        }
    }

    void Start() {
        Erase();
        Active = true;

        // Add entities to struct and compute first attacker
        float MaxSpeed = 0.0f;
        for (int i = 0; i < State->Entities.Count; i++) {
            game_entity* Entity = &State->Entities.List[i];
            combatant EntityCombatant;
            bool IsEnemy = Entity->Type == Entity_Type_Enemy;
            bool IsCharacter = Entity->Type == Entity_Type_Character;
            if (IsEnemy || IsCharacter) {
                if (IsEnemy) {
                    enemy* Enemy = &State->Enemies.List[Entity->Index];
                    EntityCombatant = Combatant(Enemy);
                }
                else if (IsCharacter) {
                    character* Character = &State->Characters.List[Entity->Index];
                    EntityCombatant = Combatant(Character);
                }

                EntityCombatant.Index = Combatants.Count;
                combatant* Combatant = &Combatants.Content[EntityCombatant.Index];
                Append(&Combatants, EntityCombatant);
                Turn.ATB[Combatant->Index] = Combatant->ATB;

                if (EntityCombatant.Stats->Speed > MaxSpeed) {
                    Turn.Attacker = Combatant;
                    MaxSpeed = EntityCombatant.Stats->Speed;
                }
                else if (Combatant->Stats->Speed == MaxSpeed) {
                    float Random = (float)rand() / (float)RAND_MAX;
                    if (Random >= 0.5f) {
                        Turn.Attacker = Combatant;
                        MaxSpeed = EntityCombatant.Stats->Speed;
                    }
                }
            }
        }

        Turn.Index = 0;
        Turn.ATBCost = 50.0f;
        Turn.nTargets = 1;
        Turn.TargetsSelected = false;

        FillTurnBuffer();
    }

    void EndTurn() {
        bool UpdateTurnBuffer = false;
        for (int i = 0; i < Turn.nTargets; i++) {
            combatant* Target = Turn.Targets[i];
            ReceiveDamage(Target, Turn.Attacker->Stats->Strength);
            // Someone died
            if (Target->Entity->Active && !IsAlive(Target)) {
                UpdateTurnBuffer = true;
                Target->Entity->Active = false;
                RemoveEntity(State, Target->Entity->ID);
            }
        }

        if (UpdateTurnBuffer) {
            FillTurnBuffer();
        }

        turn* History = PushStruct(&TurnsArena, turn);
        *History = Turn;
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

    void Update(game_input* Input) {
        Assert(Active);

        combatant* Hot = NULL;
        for (int i = 0; i < Combatants.Count; i++) {
            combatant* Combatant = &Combatants.Content[i];
            if (IsAlive(Combatant) && Combatant->Entity->Hovered) {
                Hot = Combatant;
            }
        }

        if (Hot != NULL && Input->Mouse.LeftClick.JustPressed && Hot->Type != Turn.Attacker->Type) {
            Turn.nTargets = 1;
            Turn.Targets[0] = Hot;
            EndTurn();
        }

        bool CombatEnd = true;
        for (int i = 0; i < Combatants.Count; i++) {
            combatant* Enemy = &Combatants.Content[i];
            if (Enemy->Type == Combatant_Type_Enemy) {
                if (IsAlive(Enemy)) {
                    CombatEnd = false;
                    break;
                }
            }
        }
        if (CombatEnd) End();
    }
};

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Game state                                                                                                                                   |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

enum game_state_type {
    Game_State_Main_Menu,
    Game_State_Playing,
    Game_State_Credits
};

struct game_state {
    game_entity_state Entities;
    game_combat Combat;
    double dt;
    float Time;
    game_state_type Type;
    bool Exit;
};

void Transition(game_state* State, game_state_type Type) {
    State->Type = Type;
}

void UpdateGameState(game_assets* Assets, game_state* State, game_input* Input, camera** pActiveCamera, float Width, float Height) {
    TIMED_BLOCK;
    game_entity_state* EntityState = &State->Entities;
    game_combat* Combat = &State->Combat;
    uint32 Index = 0;

// Combat
    if (!Combat->Active) {
        if (Input->Keyboard.One.JustPressed) {
            uint32 nEnemies = 3;
            v3 Position = V3(10, 0, -5 * ((nEnemies - 1) / 2.0f));
            for (int i = 0; i < nEnemies; i++) {
                AddEnemy(EntityState, Position, RandomEnemyType());
                Position.Z += 5.0f;
            }
            Combat->Start();
        }
    }
    else {
        Combat->Update(Input);
    }

// Cameras _________________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nCameras = EntityState->Cameras.Count;
    while(nCameras > 0) {
        camera* Cam = &EntityState->Cameras.List[Index++];
        game_entity* Entity = (game_entity*)Cam->Entity;

        if (Cam->OnAir) {
            *pActiveCamera = Cam;
            EntityState->ActiveCamera = Cam;
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

    // Rotation
        Cam->Basis = GetCameraBasis(Cam->Angle, Cam->Pitch);
        quaternion Rotation = Quaternion(Cam->Angle * Degrees, V3(0,1,0)) * Quaternion(Cam->Pitch * Degrees, V3(-1,0,0));
        transform Test = Transform(Rotation);
        matrix4 MatrixT = Matrix(Test);

        break;
    }

    camera* ActiveCamera = EntityState->ActiveCamera;
    
// Characters ______________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nCharacters = EntityState->Characters.Count;
    for (int i = 0; i < EntityState->Characters.Count; i++) {
        character* Character = &EntityState->Characters.List[i];
        
        Character->Entity->Velocity = V3(0, 0, 0);

        character_action_id PastAction = Character->Action.ID;
        Character->Action = GetCharacterAction(Character, Input);
        character_action_id NewAction = Character->Action.ID;
        Character->Animator.Animation = GetAsset(Assets, Character->Action.AnimationID);

        if (PastAction == Character_Action_Jump_ID) {
            Character->Entity->Collider.Capsule.Segment.Head += V3(0,Character->Armature.Bones[0].Transform.Translation.Y,0);
            Character->Entity->Collider.Capsule.Segment.Tail += V3(0,Character->Armature.Bones[0].Transform.Translation.Y,0);
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
            Character->Entity->Velocity = Speed * Direction;
            Character->Entity->Transform.Rotation = Quaternion(ActiveCamera->Angle * Degrees + Angle, V3(0,1,0));
        }
        Character->Entity->Transform.Translation += State->dt * Character->Entity->Velocity;

        if (PastAction == Character_Action_Attack_ID && NewAction == Character_Action_Idle_ID) {
            Character->Entity->Transform.Translation += Character->Entity->Transform.Rotation * V3(0,0,2);
        }
        
        Character->Entity->Collided = false;
        Character->Entity->Collider.Capsule.Segment = { V3(0,0.75f,0), V3(0,3.75f,0) };

        EntityState->ControlledCharacter = Character;

        Update(&Character->Animator);
    }

    character* ControlledCharacter = EntityState->ControlledCharacter;
    
// Autofollow player _______________________________________________________________________________________________________________________
    if (ControlledCharacter != NULL && ControlledCharacter->Entity != NULL) {
        v3 Displacement = ControlledCharacter->Entity->Transform.Translation - ActiveCamera->Position;
        Displacement.Y = 0;
        float Distance = modulus(Displacement);
        v3 Velocity = V3(0,0,0);
        float MinDistance = .01f;
        if (Distance >= MinDistance) Velocity = 20.0f * (Distance - MinDistance) * normalize(Displacement);
        ActiveCamera->Position += State->dt * Velocity;
        game_entity* ActiveCameraEntity = (game_entity*)ActiveCamera->Entity;
        ActiveCameraEntity->Transform.Translation = V3(0,0,ActiveCamera->Distance) - ActiveCamera->Position * ActiveCamera->Basis;
    }

// Enemies _________________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nEnemies = EntityState->Enemies.Count;
    while (nEnemies > 0) {
        enemy* pEnemy = &EntityState->Enemies.List[Index++];
        if (pEnemy->Entity != NULL) nEnemies--;
        else continue;

        if (pEnemy->Type == Enemy_Type_Horns) {
            pEnemy->Entity->Transform.Translation.Y = 3.2 + sin(3 * State->Time);
        }

        v3 FacingDirection = V3(-1,0,0);
        if (ControlledCharacter != NULL && ControlledCharacter->Entity != NULL) {
            FacingDirection = ControlledCharacter->Entity->Transform.Translation - pEnemy->Entity->Transform.Translation;
        }
        float Angle = atan2f(FacingDirection.Z, FacingDirection.X);
        pEnemy->Entity->Transform.Rotation = Quaternion(Angle, V3(0,1,0));
    }

// Weapons _________________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nWeapons = EntityState->Weapons.Count;
    while (nWeapons > 0) {
        weapon* pWeapon = &EntityState->Weapons.List[Index++];
        if (pWeapon->Entity != NULL) nWeapons--;
        else continue;

        if (pWeapon->ParentBone == -1) {
            pWeapon->Entity->Transform.Rotation = Quaternion(State->Time, V3(0,1,0));
        }

        pWeapon->Entity->Collided = false;

        if (ControlledCharacter != NULL && ControlledCharacter->Entity != NULL) {
            bool Collision = Collide(pWeapon->Entity, ControlledCharacter->Entity);
            if (pWeapon->Entity->Parent == NULL && Collision) {
                ControlledCharacter->Entity->Collided = true;
                pWeapon->Entity->Collided = true;
                Equip(pWeapon, ControlledCharacter);
            }
    
            if (pWeapon->ParentBone > 0) {
                bone Bone = ControlledCharacter->Armature.Bones[pWeapon->ParentBone];
                transform ModelTransform;
                if (pWeapon->Type == Weapon_Sword) {
                    ModelTransform = Transform(
                        V3(0.5f,2.0f,0),
                        Quaternion(-0.25f * Tau, V3(0,1,0)) * Quaternion(-0.25f * Tau, V3(1,0,0)),
                        pWeapon->Entity->Transform.Scale
                    );
                }
                else if (pWeapon->Type == Weapon_Shield) {
                    ModelTransform = Transform(
                        V3(-0.7f,2.2f,0),
                        Quaternion(0.5f * Tau, V3(0,0,1)) * Quaternion(0.25f * Tau, V3(1,0,0)),
                        pWeapon->Entity->Transform.Scale
                    );
                }
                pWeapon->Entity->Transform = ModelTransform * Bone.Transform * ControlledCharacter->Entity->Transform;
            }
        }
    }
}

void PushEntities(render_group* Group, game_state* GameState, game_input* Input, float Time) {
    TIMED_BLOCK;

    game_combat* Combat = &GameState->Combat;
    game_entity_state* State = &GameState->Entities;

    basis Basis = Group->Camera->Basis;
    ray Ray = MouseRay(Group->Width, Group->Height, Group->Camera->Position + Group->Camera->Distance * Basis.Z, Basis, Input->Mouse.Cursor);
    int i = 0;
    int nEntities = State->Entities.Count;
    while (nEntities > 0 && i < MAX_ENTITIES) {
        game_entity* Entity = &State->Entities.List[i++];

        if (Entity->Active) nEntities--;
        else continue;

        collider Collider = Entity->Transform * Entity->Collider;
        Entity->Hovered = Raycast(Ray, Collider);
        switch(Entity->Type) {
            case Entity_Type_Character: {
                character* pCharacter = &State->Characters.List[Entity->Index];
                PushMesh(
                    Group,
                    Mesh_Body_ID,
                    Entity->Transform,
                    Shader_Pipeline_Mesh_Bones_ID,
                    Bitmap_Empty_ID,
                    White,
                    &pCharacter->Armature,
                    Entity->Hovered
                );

                if (Combat->Active) {
                    float HPBarWidth = 2.0f;
                    float HPBarHeight = 0.2f;
                    v3 Position = Entity->Transform.Translation - 0.5f * HPBarWidth * Group->Camera->Basis.X + V3(0, 4.75f, 0);
                    PushFillbar(
                        Group, 
                        Entity->Name, 
                        pCharacter->Stats.HP, pCharacter->Stats.MaxHP,
                        Position, 
                        Group->Camera->Basis.X, Group->Camera->Basis.Y,
                        2.0f, 0.2f
                    );
                }
            } break;
    
            case Entity_Type_Enemy: {
                enemy* pEnemy = &State->Enemies.List[Entity->Index];
                PushMesh(
                    Group,
                    pEnemy->MeshID,
                    Entity->Transform,
                    Shader_Pipeline_Mesh_ID,
                    pEnemy->TextureID,
                    White, 0,
                    Entity->Hovered
                );

                if (Combat->Active) {
                    float HPBarWidth = 2.0f;
                    float HPBarHeight = 0.2f;
                    v3 Position = Entity->Transform.Translation - 0.5f * HPBarWidth * Group->Camera->Basis.X;
                    Position.Y = 4.75f;
                    PushFillbar(
                        Group, 
                        Entity->Name, 
                        pEnemy->Stats.HP, pEnemy->Stats.MaxHP,
                        Position, 
                        Group->Camera->Basis.X, Group->Camera->Basis.Y,
                        2.0f, 0.2f
                    );
                }
            } break;

            case Entity_Type_Prop: {
                prop* pProp = &State->Props.List[Entity->Index];
                PushMesh(
                    Group,
                    pProp->MeshID,
                    Entity->Transform,
                    pProp->Shader,
                    Bitmap_Empty_ID,
                    pProp->Color
                );
            } break;

            case Entity_Type_Weapon: {
                weapon* pWeapon = &State->Weapons.List[Entity->Index];
                game_mesh_id MeshID;
                switch(pWeapon->Type) {
                    case Weapon_Sword: MeshID = Mesh_Sword_ID; break;
                    case Weapon_Shield: MeshID = Mesh_Shield_ID; break;
                    default: Assert(false);
                }

                PushMesh(Group, MeshID, Entity->Transform, Shader_Pipeline_Mesh_ID);
            } break;
        }

        if (Group->Debug && Group->DebugColliders && Entity->Type != Entity_Type_Camera) {
            PushCollider(Group, Entity->Collider, Entity->Transform, Entity->Collided ? Red : Yellow);
        }
    }
}

#endif