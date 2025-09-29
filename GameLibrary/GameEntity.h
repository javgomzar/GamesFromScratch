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
// +----------------------------------------------------------------------------------------------------------------------------------------------+

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

const int MAX_CAMERAS = 16;
DefineFreeList(MAX_CAMERAS, camera);

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
// | Magic                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(magic_affinity,
    Magic_Affinity_None,
    Magic_Affinity_Fire,
    Magic_Affinity_Earth,
    Magic_Affinity_Water,
    Magic_Affinity_Ice,
    Magic_Affinity_Air,
    Magic_Affinity_Death,
    Magic_Affinity_Life,
    Magic_Affinity_Time
);

ENUM(spell_id,
    Spell_Empty,

// Fire (Tetrahedron)
    Spell_Fireball,
    Spell_Burn,
    Spell_Incinerate,
    Spell_Explosion,

// Earth (Cube)
    Spell_Rock_Throw,
    Spell_Spikes,
    Spell_Rock_Armor,
    Spell_Diamond_Armor,
    Spell_Sand_Storm,
    Spell_Sand_Burst,
    Spell_Mud_Pack,
    Spell_Earthquake,

// Water / Ice (Icosahedron)
    Spell_Drench,
    Spell_Wave,
    Spell_Drown,
    Spell_Cascade,
    Spell_Wash,
    Spell_Hydrate,

    Spell_Icicle,
    Spell_Blizzard,
    Spell_Freeze,
    Spell_Ice_Armor,
    Spell_Snow_Golem,
    Spell_Absolute_Zero,
    
// Air (Octahedron)
    Spell_Wind,
    Spell_Gust,
    Spell_Fly,
    Spell_Air_Shield,
    Spell_Tornado,
    Spell_Hurricane,

// Death / Life (Dodecahedron)
    Spell_Poison,
    Spell_Rot,
    Spell_Bleed,
    Spell_Kill,
    Spell_Zombie,
    Spell_Multikill,

    Spell_Cure,
    Spell_Heal,
    Spell_Regeneration,
    Spell_Multicure,
    Spell_Multiheal,
    Spell_Resurrect,

// Time (Circle)
    Spell_Slow,
    Spell_Accelerate,
    Spell_Tempo,
    Spell_Stop,
    Spell_Rewind
);

struct spell {
    const char* Name;
    spell_id ID;
    uint32 Damage;
    uint32 ATBCost;
    uint32 ManaCost;
    magic_affinity Affinity;
    uint8 Duration;
};

const spell Spells[spell_id_count] = {
    {},

//  Name ------------- ID ------------- Damage ATB Mana Affinity ---------- Duration
    { "Fireball",      Spell_Fireball,      20, 20, 20, Magic_Affinity_Fire,  1 },
    { "Burn",          Spell_Burn,           5, 10, 20, Magic_Affinity_Fire,  10 },
    { "Incinerate",    Spell_Incinerate,    50, 30, 30, Magic_Affinity_Fire,  1 },
    { "Explosion",     Spell_Explosion,    100, 80, 40, Magic_Affinity_Fire,  1 },

    { "Rock Throw",    Spell_Rock_Throw,    20, 20, 20, Magic_Affinity_Earth, 1 },
    { "Spikes",        Spell_Spikes,        20, 20, 20, Magic_Affinity_Earth, 1 },
    { "Rock Armor",    Spell_Rock_Armor,    20, 20, 20, Magic_Affinity_Earth, 1 },
    { "Diamond Armor", Spell_Diamond_Armor, 20, 20, 20, Magic_Affinity_Earth, 1 },
    { "Sand Storm",    Spell_Sand_Storm,    20, 20, 20, Magic_Affinity_Earth, 1 },
    { "Sand Burst",    Spell_Sand_Burst,    20, 20, 20, Magic_Affinity_Earth, 1 },
    { "Mud Pack",      Spell_Mud_Pack,      20, 20, 20, Magic_Affinity_Earth, 1 },
    { "Earthquake",    Spell_Earthquake,    20, 20, 20, Magic_Affinity_Earth, 1 },

    { "Drench",        Spell_Drench,        20, 20, 20, Magic_Affinity_Water, 1 },
    { "Wave",          Spell_Wave,          10, 20, 20, Magic_Affinity_Water, 1 },
    { "Drown",         Spell_Drown,          5, 20, 20, Magic_Affinity_Water, 10 },
    { "Cascade",       Spell_Cascade,       40, 20, 20, Magic_Affinity_Water, 1 },
    { "Wash",          Spell_Wash,           0, 20, 20, Magic_Affinity_Water, 1 },
    { "Hydrate",       Spell_Hydrate,        0, 20, 20, Magic_Affinity_Water, 1 },

    { "Icicle",        Spell_Icicle,        20, 20, 20, Magic_Affinity_Ice,   1 },
    { "Blizzard",      Spell_Blizzard,      20, 20, 20, Magic_Affinity_Ice,   1 },
    { "Freeze",        Spell_Freeze,        20, 20, 20, Magic_Affinity_Ice,   1 },
    { "Ice Armor",     Spell_Ice_Armor,     20, 20, 20, Magic_Affinity_Ice,   1 },
    { "Snow Golem",    Spell_Snow_Golem,    20, 20, 20, Magic_Affinity_Ice,   40 },
    { "Absolute Zero", Spell_Absolute_Zero, 20, 20, 20, Magic_Affinity_Ice,   1 },

    { "Wind",          Spell_Wind,          20, 20, 20, Magic_Affinity_Air,   10 },
    { "Gust",          Spell_Gust,          20, 20, 20, Magic_Affinity_Air,   1 },
    { "Fly",           Spell_Fly,           20, 20, 20, Magic_Affinity_Air,   1 },
    { "Air Shield",    Spell_Air_Shield,    20, 20, 20, Magic_Affinity_Air,   1 },
    { "Tornado",       Spell_Tornado,       20, 20, 20, Magic_Affinity_Air,   1 },
    { "Hurricane",     Spell_Hurricane,     20, 20, 20, Magic_Affinity_Air,   1 },

    { "Poison",        Spell_Poison,        20, 20, 20, Magic_Affinity_Death, 1 },
    { "Rot",           Spell_Rot,           20, 20, 20, Magic_Affinity_Death, 1 },
    { "Bleed",         Spell_Bleed,         20, 20, 20, Magic_Affinity_Death, 1 },
    { "Kill",          Spell_Kill,          20, 20, 20, Magic_Affinity_Death, 1 },
    { "Zombie",        Spell_Zombie,        20, 20, 20, Magic_Affinity_Death, 1 },
    { "Multikill",     Spell_Multikill,     20, 20, 20, Magic_Affinity_Death, 1 },

    { "Cure",          Spell_Cure,          20, 20, 20, Magic_Affinity_Life,  1 },
    { "Heal",          Spell_Heal,          20, 20, 20, Magic_Affinity_Life,  1 },
    { "Regeneration",  Spell_Regeneration,  20, 20, 20, Magic_Affinity_Life,  1 },
    { "Multicure",     Spell_Multicure,     20, 20, 20, Magic_Affinity_Life,  1 },
    { "Multiheal",     Spell_Multiheal,     20, 20, 20, Magic_Affinity_Life,  1 },
    { "Resurrect",     Spell_Resurrect,     20, 20, 20, Magic_Affinity_Life,  1 },

    { "Slow",          Spell_Slow,          20, 20, 20, Magic_Affinity_Time,  1 },
    { "Accelerate",    Spell_Accelerate,    20, 20, 20, Magic_Affinity_Time,  1 },
    { "Tempo",         Spell_Tempo,         20, 20, 20, Magic_Affinity_Time,  1 },
    { "Stop",          Spell_Stop,          20, 20, 20, Magic_Affinity_Time,  1 },
    { "Rewind",        Spell_Rewind,        20, 20, 20, Magic_Affinity_Time,  1 },
};

const int nFireSpells = 4;
const spell_id FireSpellIDs[nFireSpells] = { Spell_Fireball, Spell_Burn, Spell_Incinerate, Spell_Explosion };

const int nEarthSpells = 8;
const spell_id EarthSpellIDs[nEarthSpells] = { Spell_Rock_Throw, Spell_Spikes, Spell_Rock_Armor, Spell_Diamond_Armor, Spell_Sand_Storm, Spell_Sand_Burst, Spell_Mud_Pack, Spell_Earthquake, };

const int nWaterIceSpells = 12;
const spell_id WaterIceSpellIDs[nWaterIceSpells] = { Spell_Drench, Spell_Wave, Spell_Drown, Spell_Cascade, Spell_Wash, Spell_Hydrate, Spell_Icicle, Spell_Blizzard, Spell_Freeze, Spell_Ice_Armor, Spell_Snow_Golem, Spell_Absolute_Zero };

const int nAirSpells = 6;
const spell_id AirSpellIDs[nAirSpells] = { Spell_Wind, Spell_Gust, Spell_Fly, Spell_Air_Shield, Spell_Tornado, Spell_Hurricane };

const int nLifeDeathSpells = 12;
const spell_id LifeDeathSpellIDs[nLifeDeathSpells] = { Spell_Poison, Spell_Rot, Spell_Bleed, Spell_Kill, Spell_Zombie, Spell_Multikill, Spell_Cure, Spell_Heal, Spell_Regeneration, Spell_Multicure, Spell_Multiheal, Spell_Resurrect, };

const int nTimeSpells = 5;
const spell_id TimeSpellIDs[nTimeSpells] = { Spell_Slow, Spell_Accelerate, Spell_Tempo, Spell_Stop, Spell_Rewind, };

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Enemies                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(enemy_type,
    Enemy_Type_Horns,
    Enemy_Type_Dog,
    Enemy_Type_Miniboss_Dyno,
    Enemy_Type_Boss_Test
);

enemy_type EnemyTypeFirstMiniboss = Enemy_Type_Miniboss_Dyno;
enemy_type EnemyTypeFirstBoss = Enemy_Type_Boss_Test;

INTROSPECT
struct enemy {
    uint32 ID;
    game_entity* Entity;
    stats Stats;
    enemy_type Type;
    game_mesh_id MeshID;
    game_bitmap_id TextureID;
    bool KnownSpells[spell_id_count];
};

const enemy EnemyTemplates[enemy_type_count] = {
    {
        0,
        NULL,
        Stats(50, 7, 10, 5, 5, 6, 10),
        Enemy_Type_Horns,
        Mesh_Horns_ID,
        Bitmap_Enemy_ID,
    },
    {
        0,
        NULL,
        Stats(20, 2, 8, 2, 6, 10, 10),
        Enemy_Type_Dog,
        Mesh_Dog_ID,
        Bitmap_Empty_ID,
    },
    {
        0,
        NULL,
        Stats(100, 10, 8, 1, 3, 5, 10),
        Enemy_Type_Miniboss_Dyno,
        Mesh_Dyno_ID,
        Bitmap_Empty_ID,
    },
};

collider EnemyColliders[enemy_type_count] = {
    SphereCollider(V3(0,0,0), 1.5f),
    SphereCollider(V3(0,1.5f,0), 2.0f),
    SphereCollider(V3(0,2.5f,0), 4.0f),
};

const char* EnemyNames[enemy_type_count] = {
    "Horns",
    "Dog",
    "Dyno"
};

void FillTemplate(enemy* Enemy, enemy_type Type) {
    enemy Template = EnemyTemplates[Type];
    Enemy->MeshID = Template.MeshID;
    Enemy->Stats = Template.Stats;
    Enemy->TextureID = Template.TextureID;
    Enemy->Type = Type;
}

const int MAX_ENEMIES = 32;
DefineFreeList(MAX_ENEMIES, enemy);

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Weapons                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

/*
    Weapon ideas:
        - Chest: Allows you to carry a lot of items but damage is proportional to how many items it carries. Bad at start, good later.
        - Bow: Model done
        - Crossbows
        - Knife: Low ATB cost, high critical rate?
        - Axe: Slow, good DPS can cause bleeding
        - Spear: Attacks when defending
        - Scimitar: Sword, but better critical rate?
        - Hammers / Clubs
        - Morning star
*/

ENUM(weapon_type,
    Weapon_Sword,
    Weapon_Shield,
    Weapon_Staff,
    Weapon_Bow,
    Weapon_Knife
);

game_mesh_id WeaponMeshIDs[weapon_type_count] = {
    Mesh_Sword_ID,
    Mesh_Shield_ID,
    Mesh_Staff_ID,
    Mesh_Bow_ID,
    Mesh_Knife_ID
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
    Transform(
        V3(0.4f,2.0f,1.0f),
        Quaternion(-0.25f * Tau, V3(0,1,0)) * Quaternion(-0.25f * Tau, V3(1,0,0)),
        Scale(0.75, 0.75, 0.75)
    ),
    Transform(
        V3(0.5f,2.0f,0),
        Quaternion(0.25f * Tau, V3(0,1,0)) * Quaternion(-0.25f * Tau, V3(1,0,0))
    ),
    Transform(
        V3(0.5f,2.0f,0),
        Quaternion(-0.25f * Tau, V3(0,1,0)) * Quaternion(-0.25f * Tau, V3(1,0,0))
    ),
};

collider WeaponColliders[weapon_type_count] = {
    CapsuleCollider(V3(0,0,0), V3(0,3,0), 0.5f),
    CapsuleCollider(V3(0,-0.3,0), V3(0,0.7,0), 1.0f),
    CapsuleCollider(V3(0,-4.5,0), V3(0,2,0), 0.5f),
    CapsuleCollider(V3(0,0,0), V3(0,3,0), 0.5f),
    CapsuleCollider(V3(0,0,0), V3(0,3,0), 0.5f),
};

INTROSPECT
struct weapon {
    uint32 ID;
    weapon_type Type;
    magic_affinity Affinity;
    color Color;
    game_entity* Entity;
    int ParentBone;
    bool SpellCasting;
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

INTROSPECT
struct character_action {
    character_action_id ID;
    game_animation_id AnimationID;
    bool Loop;
};

ENUM(character_class,
    Class_Knight,
    Class_Rogue,
    Class_Hunter,
    Class_Wizard
);

const char* ClassNames[character_class_count] = {
    "Knight",
    "Rogue",
    "Hunter",
    "Wizard",
};

INTROSPECT
struct character {
    uint32 ID;
    armature Armature;
    stats Stats;
    game_animator Animator;
    game_entity* Entity;
    weapon* LeftHand;
    weapon* RightHand;
    character_action Action;
    character_class Class;
    bool KnownSpells[spell_id_count];
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
    if (Weapon->Type == Weapon_Shield) {
        Character->LeftHand = Weapon;
        Weapon->ParentBone = 2;
    }
    else {
        Character->RightHand = Weapon;
        Weapon->ParentBone = 8;
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
    game_shader_pipeline_id Shader;
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

    Remove(&State->Entities, Entity->ID);
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

weapon* AddWeapon(
    game_entity_state* State,
    weapon_type Type,
    color Color = White,
    v3 Position = V3(0,0,0),
    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0),
    scale S = Scale()
) {
    Assert(State->Weapons.Count < MAX_WEAPONS);

    weapon* pWeapon = Insert(&State->Weapons);
    pWeapon->Type = Type;
    pWeapon->ParentBone = -1;
    pWeapon->Color = Color;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Weapon %d", pWeapon->ID);

    collider Collider = WeaponColliders[pWeapon->Type];
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

character* AddCharacter(game_entity_state* State, character_class Class, v3 Position, int MaxHP) {
    Assert(State->Characters.Count < MAX_CHARACTERS);

    character* pCharacter = Insert(&State->Characters);

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Character %d", pCharacter->ID);

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
    pCharacter->Entity->Index = pCharacter->ID;

    pCharacter->Stats.MaxHP = MaxHP;
    pCharacter->Stats.HP = MaxHP;
    pCharacter->Stats.Strength = 10;
    pCharacter->Stats.Defense = 10;
    pCharacter->Stats.Intelligence = 10;
    pCharacter->Stats.Wisdom = 10;
    pCharacter->Stats.Speed = 10;
    pCharacter->Stats.Precission = 10;

    switch (Class) {
        case Class_Knight: {
            weapon* Sword = AddWeapon(State, Weapon_Sword, White, V3(-5,0,0));
            weapon* Shield = AddWeapon(State, Weapon_Shield, White, V3(-10,0,0));
            Equip(Sword, pCharacter);
            Equip(Shield, pCharacter);
        } break;

        case Class_Rogue: {
            weapon* Knife = AddWeapon(State, Weapon_Knife, White, V3(-5,0,0));
            Equip(Knife, pCharacter);
        } break;
        
        case Class_Hunter: {
            weapon* Bow = AddWeapon(State, Weapon_Bow, White, V3(-5,0,0));
            Equip(Bow, pCharacter);
        } break;

        case Class_Wizard: {
            weapon* Staff = AddWeapon(State, Weapon_Staff, White, V3(-5,0,0));
            Equip(Staff, pCharacter);
        } break;
    }

    return pCharacter;
}

enemy* AddEnemy(game_entity_state* State, v3 Position, enemy_type Type) {
    Assert(State->Characters.Count < MAX_ENEMIES);
    static int32 EnemyQuantities[enemy_type_count] = {};

    // If any ID is free, use it
    enemy* pEnemy = Insert(&State->Enemies);
    FillTemplate(pEnemy, Type);

    char NameBuffer[32];
    sprintf_s(NameBuffer, "%s %d", EnemyNames[Type], EnemyQuantities[Type]++);

    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
    collider Collider = EnemyColliders[Type];
    pEnemy->Entity = AddEntity(State, NameBuffer, Entity_Type_Enemy, Collider, Position, Rotation, Scale());
    pEnemy->Entity->Index = pEnemy->ID;
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

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Combat                                                                                                                                       |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(altered_state,
    altered_state_none,
    altered_state_burned,
    altered_state_wet,
    altered_state_frozen,
    altered_state_drowning,
    altered_state_poisoned,
    altered_state_rotting,
    altered_state_bleeding,
    altered_state_dead,
    altered_state_regenerating
);

struct damage_animation {
    uint32 ID;
    uint32 Damage;
    float t;
    bool Active;
};

DefineFreeList(16, damage_animation);

void Update(damage_animation_list* CombatAnimations, render_group* Group, float dt) {
    char TextBuffer[32];

    uint32 Index = 0;
    uint32 nAnimations = CombatAnimations->Count;
    while (nAnimations > 0) {
        damage_animation* Animation = &CombatAnimations->List[Index];
        if (Animation->Active) nAnimations--;
        else {
            Index++;
            continue;
        }

        if (Animation->t > 1.0f) {
            Remove(CombatAnimations, Index);
        }
        else {
            Animation->t += dt;
            sprintf_s(TextBuffer, "%u", Animation->Damage);
            PushText(Group, V2(300, 300), Font_Menlo_Regular_ID, TextBuffer);
        }

        Index++;
    }
}

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
    bool KnownSpells[spell_id_count];
    bool State[altered_state_count];
};

combatant Combatant(character* Character) {
    combatant Result;
    Result.Stats = &Character->Stats;
    Result.Entity = Character->Entity;
    Result.ATB = 100.0f;
    Result.Type = Combatant_Type_Player;
    for (int i = 0; i < spell_id_count; i++) {
        Result.KnownSpells[i] = Character->KnownSpells[i];
    }
    return Result;
}

combatant Combatant(enemy* Enemy) {
    combatant Result;
    Result.Stats = &Enemy->Stats;
    Result.Entity = Enemy->Entity;
    Result.ATB = 100.0f;
    Result.Type = Combatant_Type_Enemy;
    for (int i = 0; i < spell_id_count; i++) {
        Result.KnownSpells[i] = Enemy->KnownSpells[i];
    }
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
    combatant_action_empty,
    combatant_action_attack,
    combatant_action_magic,
    combatant_action_items,
    combatant_action_flee,

    combatant_action_count
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
    spell_id Spell;
    bool TargetsSelected;
};

ArrayDefinition(MAX_COMBATANTS, combatant);

const int TURN_BUFFER_SIZE = 16;
struct game_combat {
    combatant_array Combatants;
    turn NextTurns[TURN_BUFFER_SIZE];
    turn Turn;
    memory_arena TurnsArena;
    damage_animation_list DamageAnimations;
    game_entity_state* State;
    render_group* Group;
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
        Result.Action = combatant_action_empty;
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
        uint32 nEntities = State->Entities.Count;
        uint32 Index = 0;
        while(nEntities > 0 && Index < MAX_ENTITIES) {
            game_entity* Entity = &State->Entities.List[Index++];
            if (!Entity->Active) continue;
            else nEntities--;
            
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
                    if (Bernoulli()) {
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

            uint32 Damage = 0;
            switch (Turn.Action) {
                case combatant_action_attack: {
                    Damage = Turn.Attacker->Stats->Strength;
                } break;
                case combatant_action_magic: {
                    spell Spell = Spells[Turn.Spell];
                    Damage = Spell.Damage;
                } break;
            }

            if (Damage > 0) {
                ReceiveDamage(Target, Damage);
            }

            damage_animation* Animation = Insert(&DamageAnimations);
            Animation->Active = true;
            Animation->Damage = Damage;
            Animation->t = 0;

            // Did someone die?
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
};

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Rooms                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(room_type,
    Room_Type_Combat,
    Room_Type_Camp,
    Room_Type_Merchant,
    Room_Type_Blacksmith,
    Room_Type_Wizard,
    Room_Type_Quest,
    Room_Type_Miniboss,
    Room_Type_Boss
);

const int MAX_ROOM_NEXT_LINKS = 4;
const int MAX_ROOM_PREVIOUS_LINKS = 8;
struct room {
    room_type Type;
    room* Next[MAX_ROOM_NEXT_LINKS];
    room* Previous[MAX_ROOM_PREVIOUS_LINKS];
    uint8 nNext;
    uint8 nPrevious;
    uint8 Depth;
    uint8 RowIndex;
};

room NewRoom(room_type Type) {
    room Result = {};
    Result.Type = Type;

    return Result;
}

room_type RandomRoomType(bool NoCombat = false) {
    float Rand = RandFloat();

    if (NoCombat) {
        return (room_type)RandInt(1, 6);
    }

    if (Rand < 0.5f) {
        return Room_Type_Combat;
    }

    if (Rand > 0.95f) {
        return Room_Type_Miniboss;
    }

    return (room_type)RandInt(1, 6);
}

const uint32 MAX_LEVEL_ROOMS = 32;
const uint32 MAX_LEVEL_DEPTH = 16;
struct level {
    room Rooms[MAX_LEVEL_ROOMS];
    uint32 nRooms;
    uint32 Depth;
    uint32 nInRow[MAX_LEVEL_DEPTH];
};

room* AddRoom(level* Level, room_type RoomType) {
    Assert(Level->nRooms < MAX_LEVEL_ROOMS);
    room* Room = &Level->Rooms[Level->nRooms++];
    *Room = NewRoom(RoomType);
    Room->Depth = Level->Depth;
    Room->RowIndex = Level->nInRow[Level->Depth]++;
    return Room;
}

void NextRow(level* Level) {
    Level->Depth++;
}

void AttachRooms(room* Previous, room* Next) {
    Assert(Previous->nNext < MAX_ROOM_NEXT_LINKS && Next->nPrevious < MAX_ROOM_PREVIOUS_LINKS);
    Previous->Next[Previous->nNext++] = Next;
    Next->Previous[Next->nPrevious++] = Previous;
}

void RandomizeLevel(level* Level) {
    *Level = {};
    Level->nRooms = 0;

    room* FirstRoom = AddRoom(Level, Room_Type_Combat);

    NextRow(Level);
    for (int i = 0; i < 4; i++) {
        room* Room = AddRoom(Level, RandomRoomType());

        AttachRooms(FirstRoom, Room);
    }
    
    int PreviousFirst = 1;
    NextRow(Level);
    int nPreviousRow = Level->nInRow[1];
    for (int j = 0; j < nPreviousRow; j++) {
        room* Previous = &Level->Rooms[PreviousFirst + j];
        room* Next = AddRoom(Level, RandomRoomType());
        AttachRooms(Previous, Next);
    }
    PreviousFirst += nPreviousRow;

    NextRow(Level);
    nPreviousRow = Level->nInRow[2];
    for (int j = 0; j < nPreviousRow; j++) {
        room* Previous = &Level->Rooms[PreviousFirst + j];
        room* Next = AddRoom(Level, RandomRoomType());
        AttachRooms(Previous, Next);
    }
    PreviousFirst += nPreviousRow;

    NextRow(Level);
    room* LastCamp = AddRoom(Level, Room_Type_Camp);
    for (int i = PreviousFirst; i < Level->nRooms; i++) {
        room* Room = &Level->Rooms[i];
        AttachRooms(Room, LastCamp);
    }

    NextRow(Level);
    room* LastRoom = AddRoom(Level, Room_Type_Boss);
    AttachRooms(LastCamp, LastRoom);
}

void PushRoom(render_group* Group, room* Room, v2 Position) {
    game_bitmap_id Bitmap;

    switch(Room->Type) {
        case Room_Type_Combat: {
            Bitmap = Bitmap_Combat_ID;
        } break;
        case Room_Type_Camp: {
            Bitmap = Bitmap_Fire_ID;
        } break;
        case Room_Type_Merchant: {
            Bitmap = Bitmap_Coin_ID;
        } break;
        case Room_Type_Blacksmith: {
            Bitmap = Bitmap_Anvil_ID;
        } break;
        case Room_Type_Wizard: {
            Bitmap = Bitmap_Wizard_ID;
        } break;
        case Room_Type_Quest: {
            Bitmap = Bitmap_Quest_ID;
        } break;
        case Room_Type_Miniboss: {
            Bitmap = Bitmap_Miniboss_ID;
        } break;
        case Room_Type_Boss: {
            Bitmap = Bitmap_Boss_ID;
        } break;
    }

    float X = Position.X, Y = Position.Y;
    PushBitmap(Group, Bitmap, {X, Y, 100, 100});
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Game state                                                                                                                                   |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(game_state_type,
    Game_State_Main_Menu,
    Game_State_Combat,
    Game_State_Camp,
    Game_State_Trade,
    Game_State_Map,
    Game_State_Credits
);

game_state_type GetStateType(room_type RoomType) {
    switch(RoomType) {
        case Room_Type_Boss:
        case Room_Type_Miniboss:
        case Room_Type_Quest:
        case Room_Type_Combat: 
            return Game_State_Combat;
        
        case Room_Type_Merchant:
        case Room_Type_Blacksmith:
        case Room_Type_Wizard:
            return Game_State_Trade;

        case Room_Type_Camp:
            return Game_State_Camp;
    }

    Assert(false);
    return Game_State_Map;
}

struct game_state {
    game_entity_state Entities;
    particle_emitter* Emitter;
    game_combat Combat;
    level Level;
    room* CurrentRoom;
    character* ControlledCharacter;
    camera* ActiveCamera;
    double dt;
    float Time;
    float CampTime;
    game_state_type Type;
    bool Exit;
};

void Transition(game_state* State, game_state_type Type) {
    State->Type = Type;

    switch(Type) {
        case Game_State_Combat: {
            uint32 nEnemies = RandInt(2, 4);
            v3 Position = V3(10, 0, -5 * ((nEnemies - 1) / 2.0f));
            for (int i = 0; i < nEnemies; i++) {
                enemy_type EnemyType = (enemy_type)RandInt(0, EnemyTypeFirstMiniboss);
                AddEnemy(&State->Entities, Position, EnemyType);
                Position.Z += 5.0f;
            }

            if (State->CurrentRoom->Type == Room_Type_Quest) {
                character_class Companion = RandomEnum(character_class);

                AddCharacter(&State->Entities, Companion, V3(0,0,0), 500);
            }

            State->Combat.Start();
        } break;

        case Game_State_Camp: {
            State->CampTime = 0;
        } break;
    }
}

void UpdateEntities(render_group* Group, game_state* State, game_input* Input) {
    game_entity_state* EntityState = &State->Entities;
    game_combat* Combat = &State->Combat;
    uint32 Index = 0;

// Combat
    if (Combat->Active) {
        combatant* Hot = NULL;
        combatant_array* Combatants = &Combat->Combatants;
        for (int i = 0; i < Combatants->Count; i++) {
            combatant* Combatant = &Combatants->Content[i];
            if (IsAlive(Combatant) && Combatant->Entity->Hovered) {
                Hot = Combatant;
            }
        }

        turn* Turn = &Combat->Turn;
        if (Hot != NULL && Input->Mouse.LeftClick.JustPressed) {
            switch(Turn->Action) {
                case combatant_action_attack: {
                    if (Hot->Type != Combat->Turn.Attacker->Type) {
                        Turn->nTargets = 1;
                        Turn->Targets[0] = Hot;
                        Combat->EndTurn();
                    }
                } break;
                case combatant_action_magic: {
                    if (Hot->Type != Turn->Attacker->Type && Turn->Spell != Spell_Empty) {
                        Turn->nTargets = 1;
                        Turn->Targets[0] = Hot;
                        Combat->EndTurn();
                    }
                } break;
            }
        }

        Update(&Combat->DamageAnimations, Group, State->dt);

        bool CombatEnd = true;
        for (int i = 0; i < Combatants->Count; i++) {
            combatant* Enemy = &Combatants->Content[i];
            if (Enemy->Type == Combatant_Type_Enemy) {
                if (IsAlive(Enemy)) {
                    CombatEnd = false;
                    break;
                }
            }
        }
        if (CombatEnd) {
            Combat->End();
            Transition(State, Game_State_Map);
        }
    }

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
            Input->Mouse.Cursor.X >= 0 && Input->Mouse.Cursor.X <= Group->Width &&
            Input->Mouse.Cursor.Y >= 0 && Input->Mouse.Cursor.Y <= Group->Height
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
    
// Characters ______________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nCharacters = EntityState->Characters.Count;
    for (int i = 0; i < EntityState->Characters.Count; i++) {
        character* Character = &EntityState->Characters.List[i];

        if (Character->Armature.nBones == 0) {
            Character->Armature = GetAsset(Group->Assets, Mesh_Body_ID)->Armature;
            Character->Animator.Armature = &Character->Armature;
            Character->Animator.Animation = GetAsset(Group->Assets, Animation_Idle_ID);
            Character->Animator.Loop = true;
            Character->Animator.Active = true;
            Character->Action.ID = Character_Action_Idle_ID;
            Character->Action.Loop = true;
        }
        
        Character->Entity->Collided = false;

        if (Combat->Active && Combat->Turn.Attacker->Entity == Character->Entity) {
            State->ActiveCamera->Follow = Character->Entity;
            State->ControlledCharacter = Character;
        }

        Update(&Character->Animator);
    }
    
// Movement _______________________________________________________________________________________________________________________
    if (State->ControlledCharacter != NULL && State->ControlledCharacter->Entity != NULL) {
        character* Character = State->ControlledCharacter;

        // Actions
        character_action_id PastAction = Character->Action.ID;
        Character->Action = GetCharacterAction(Character, Input);
        character_action_id NewAction = Character->Action.ID;
        Character->Animator.Animation = GetAsset(Group->Assets, Character->Action.AnimationID);

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
        
        // Camera autofollow
        v3 Displacement = Character->Entity->Transform.Translation - State->ActiveCamera->Position;
        Displacement.Y = 0;
        float Distance = modulus(Displacement);
        v3 Velocity = V3(0,0,0);
        float MinDistance = .01f;
        if (Distance >= MinDistance) Velocity = 20.0f * (Distance - MinDistance) * normalize(Displacement);
        State->ActiveCamera->Position += State->dt * Velocity;
        State->ActiveCamera->Entity->Transform.Translation = 
            V3(0,0,State->ActiveCamera->Distance) - State->ActiveCamera->Position * State->ActiveCamera->Basis;
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
        if (State->ControlledCharacter != NULL && State->ControlledCharacter->Entity != NULL) {
            FacingDirection = State->ControlledCharacter->Entity->Transform.Translation - pEnemy->Entity->Transform.Translation;
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
    TIMED_BLOCK;

    game_combat* Combat = &GameState->Combat;
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
        bool Outline = Entity->Hovered && (
            Combat->Turn.Action == combatant_action_attack ||
            Combat->Turn.Action == combatant_action_magic && Combat->Turn.Spell != Spell_Empty
        );
        switch(Entity->Type) {
            case Entity_Type_Character: {
                character* pCharacter = &State->Characters.List[Entity->Index];
                game_mesh* Mesh = GetAsset(Assets, Mesh_Body_ID);
                PushMesh(
                    Group,
                    Mesh_Body_ID,
                    Entity->Transform,
                    Shader_Pipeline_Mesh_Bones_ID,
                    Bitmap_Empty_ID,
                    White,
                    &pCharacter->Armature,
                    Outline
                );

                if (Combat->Active) {
                    float HPBarWidth = 2.0f;
                    float HPBarHeight = 0.2f;
                    v3 Position = Entity->Transform.Translation - 0.5f * HPBarWidth * Camera->Basis.X + V3(0, Mesh->MaxY + 0.3f, 0);
                    PushFillbar(
                        Group, 
                        Entity->Name, 
                        pCharacter->Stats.HP, pCharacter->Stats.MaxHP,
                        Position, 
                        Camera->Basis.X, Camera->Basis.Y,
                        2.0f, 0.2f
                    );

                    if (Combat->Turn.Attacker->Entity == Entity) {
                        v3 SelectorPosition = Entity->Transform.Translation;
                        SelectorPosition.Y += 1.0f + 0.1f * sinf(5.0f * Time) + Mesh->MaxY;
                        transform T = Transform(SelectorPosition, Quaternion(Time, V3(0,1,0)));
                        PushMesh(Group, Mesh_Selector_ID, T, Shader_Pipeline_Mesh_ID, Bitmap_Empty_ID, Red);
                    }
                }
            } break;
    
            case Entity_Type_Enemy: {
                enemy* pEnemy = &State->Enemies.List[Entity->Index];
                game_mesh* Mesh = GetAsset(Assets, pEnemy->MeshID);
                PushMesh(
                    Group,
                    pEnemy->MeshID,
                    Entity->Transform,
                    Shader_Pipeline_Mesh_ID,
                    pEnemy->TextureID,
                    White, 0,
                    Outline
                );

                if (Combat->Active) {
                    float HPBarWidth = 2.0f;
                    float HPBarHeight = 0.2f;
                    v3 Position = Entity->Transform.Translation - 0.5f * HPBarWidth * Camera->Basis.X + V3(0, Mesh->MaxY + 0.3f, 0);
                    PushFillbar(
                        Group, 
                        Entity->Name, 
                        pEnemy->Stats.HP, pEnemy->Stats.MaxHP,
                        Position, 
                        Camera->Basis.X, Camera->Basis.Y,
                        2.0f, 0.2f
                    );

                    if (Combat->Turn.Attacker->Entity == Entity) {
                        v3 SelectorPosition = Entity->Transform.Translation;
                        SelectorPosition.Y += 1.0f + 0.1f * sinf(5.0f * Time) + Mesh->MaxY;
                        transform T = Transform(SelectorPosition, Quaternion(Time, V3(0,1,0)));
                        PushMesh(Group, Mesh_Selector_ID, T, Shader_Pipeline_Mesh_ID, Bitmap_Empty_ID, Red);
                    }
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
                game_mesh_id MeshID = WeaponMeshIDs[pWeapon->Type];

                PushMesh(Group, MeshID, Entity->Transform, Shader_Pipeline_Mesh_ID);
            } break;
        }

        if (Group->Debug && Group->DebugColliders && Entity->Type != Entity_Type_Camera) {
            PushCollider(Group, Entity->Collider, Entity->Transform, Entity->Collided ? Red : Yellow);
        }
    }
}

#endif