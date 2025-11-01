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
// | Stats                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

INTROSPECT
struct stats {
    uint32 HP;
    uint32 MaxHP;
    uint16 Strength;
    uint16 Precission;
    uint16 Defense;
    uint16 Intelligence;
    uint16 Wisdom;
    float Speed;
};

stats Stats(uint32 MaxHP, uint16 Strength, uint16 Precission, uint16 Defense, uint16 Intelligence, uint16 Wisdom, float Speed) {
    return {
        MaxHP, MaxHP, Strength, Precission, Defense, Intelligence, Wisdom, Speed
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

const int MAX_COMBATANT_SPELLS = 4;

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Items                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(item_type,
    Item_Type_None,
    Item_Type_Potion,
    Item_Type_Antidote,
    Item_Type_Bomb,
    Item_Type_Phoenix_Feather,
    Item_Type_Lightning_Bottle,
    Item_Type_Water_Bottle,
    Item_Type_Poison,
    Item_Type_Acid
);

struct item {
    const char* Name;
    item_type Type;
    game_bitmap_id BitmapID;
    float Rarity;
};

item Items[item_type_count] = {
    {
        "",
        Item_Type_None,
        Bitmap_Empty_ID,
        0.0f
    },
    {
        "Potion",
        Item_Type_Potion,
        Bitmap_Potion_ID,
        1.0f
    },
    {
        "Antidote",
        Item_Type_Antidote,
        Bitmap_Antidote_ID,
        1.0f
    },
    {
        "Bomb",
        Item_Type_Bomb,
        Bitmap_Bomb_ID,
        1.0f
    },
    {
        "Phoenix Feather",
        Item_Type_Phoenix_Feather,
        Bitmap_Phoenix_Feather_ID,
        0.1f
    },
    {
        "Lightning in a bottle",
        Item_Type_Lightning_Bottle,
        Bitmap_Lightning_Bottle_ID,
        0.5f
    },
    {
        "Water bottle",
        Item_Type_Water_Bottle,
        Bitmap_Water_Bottle_ID,
        1.0f
    },
    {
        "Poison",
        Item_Type_Poison,
        Bitmap_Poison_ID,
        1.0f
    },
    {
        "Acid",
        Item_Type_Acid,
        Bitmap_Acid_ID,
        1.0f
    },
};

item_type RandomItemType() {
    float TotalRarity = 0;
    for (int i = 0; i < item_type_count; i++) {
        TotalRarity += Items[i].Rarity;
    }

    float r = RandFloat();
    float Acc = 0;
    for (int i = 0; i < item_type_count; i++) {
        Acc += Items[i].Rarity / TotalRarity;
        if (r < Acc) {
            return (item_type)i;
        }
    }

    return (item_type)(item_type_count - 1);
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Enemies                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(enemy_type,
    Enemy_Type_Horns,
    Enemy_Type_Dog,
    Enemy_Type_Miniboss_Dyno,
    Enemy_Type_Boss_Squid
);

enemy_type EnemyTypeFirstMiniboss = Enemy_Type_Miniboss_Dyno;
enemy_type EnemyTypeFirstBoss = Enemy_Type_Boss_Squid;

INTROSPECT
struct enemy {
    uint32 ID;
    game_entity* Entity;
    stats Stats;
    enemy_type Type;
    game_mesh_id MeshID;
    game_bitmap_id TextureID;
    spell_id Spells[MAX_COMBATANT_SPELLS];
};

const enemy EnemyTemplates[enemy_type_count] = {
    {
        0,
        NULL,
        Stats(50, 10, 10, 5, 5, 6, 10),
        Enemy_Type_Horns,
        Mesh_Horns_ID,
        Bitmap_Enemy_ID,
    },
    {
        0,
        NULL,
        Stats(30, 20, 8, 2, 6, 10, 10),
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
    {
        0,
        NULL,
        Stats(200, 20, 8, 1, 3, 5, 10),
        Enemy_Type_Boss_Squid,
        Mesh_Squid_ID,
        Bitmap_Squid_ID,
        {
            Spell_Drench,
            Spell_Cascade,
        }
    },
};

collider EnemyColliders[enemy_type_count] = {
    SphereCollider(V3(0,0,0), 1.5f),
    SphereCollider(V3(0,1.5f,0), 2.0f),
    SphereCollider(V3(0,2.5f,0), 4.0f),
    CapsuleCollider(V3(0,4.0f,0), V3(0,12.0f,0), 6.0f),
};

const char* EnemyNames[enemy_type_count] = {
    "Horns",
    "Dog",
    "Dyno",
    "Squid"
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

INTROSPECT
struct weapon {
    weapon_type Type;
    game_mesh_id MeshID;
    collider Collider;
    transform Transform;
    bool SpellCasting;
    stats Modifier;
    uint32 ID;
    game_entity* Entity;
    magic_affinity Affinity;
    color Color;
    int ParentBone;
};

weapon WeaponTemplates[weapon_type_count] = {
    {
        Weapon_Sword,
        Mesh_Sword_ID,
        CapsuleCollider(V3(0,0,0), V3(0,3,0), 0.5f),
        Transform(
            V3(0.5f,2.0f,0),
            Quaternion(-0.25f * Tau, V3(0,1,0)) * Quaternion(-0.25f * Tau, V3(1,0,0))
        )
    },
    {
        Weapon_Shield,
        Mesh_Shield_ID,
        CapsuleCollider(V3(0,-0.3,0), V3(0,0.7,0), 1.0f),
        Transform(
            V3(-0.7f,2.2f,0),
            Quaternion(0.5f * Tau, V3(0,0,1)) * Quaternion(0.25f * Tau, V3(1,0,0))
        )
    },
    {
        Weapon_Staff,
        Mesh_Staff_ID,
        CapsuleCollider(V3(0,-4.5,0), V3(0,2,0), 0.5f),
        Transform(
            V3(0.4f,2.0f,1.0f),
            Quaternion(-0.25f * Tau, V3(0,1,0)) * Quaternion(-0.25f * Tau, V3(1,0,0)),
            Scale(0.75, 0.75, 0.75)
        ),
        true
    },
    {
        Weapon_Bow,
        Mesh_Bow_ID,
        CapsuleCollider(V3(0,0,0), V3(0,3,0), 0.5f),
        Transform(
            V3(0.5f,2.0f,0),
            Quaternion(0.25f * Tau, V3(0,1,0)) * Quaternion(-0.25f * Tau, V3(1,0,0))
        )
    },
    {
        Weapon_Knife,
        Mesh_Knife_ID,
        CapsuleCollider(V3(0,0,0), V3(0,3,0), 0.5f),
        Transform(
            V3(0.5f,2.0f,0),
            Quaternion(-0.25f * Tau, V3(0,1,0)) * Quaternion(-0.25f * Tau, V3(1,0,0))
        ),
    },
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
    Character_Action_Attack_ID,
    Character_Action_Dead_ID
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
    spell_id Spells[MAX_COMBATANT_SPELLS];
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
            Result.Loop = true;
        } break;
        case Character_Action_Dead_ID: {
            Result.AnimationID = Animation_Dead_ID;
            Result.Loop = true;
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

    if (Character->Stats.HP == 0) {
        Result = CharacterAction(Character_Action_Dead_ID);
    }
    else {
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

struct game_entity_manager {
    game_entity_list Entities;
    camera_list Cameras;
    character_list Characters;
    enemy_list Enemies;
    uint32 nEnemyTypes[enemy_type_count];
    prop_list Props;
    weapon_list Weapons;
};

game_entity* AddEntity(
    game_entity_list* Entities,
    const char* Name,
    game_entity_type Type,
    collider Collider,
    v3 Position = V3(0,0,0),
    quaternion Rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f),
    scale S = Scale(),
    bool Active = true
) {
    Assert(Entities->Count < MAX_ENTITIES);

    // If any ID is free, use it
    game_entity* Entity = Insert(Entities);
    Entity->Type = Type;
    Entity->Transform = Transform(Position, Rotation, S);
    Entity->Active = Active;
    Entity->Collider = Collider;
    Entity->Parent = NULL;
    strcpy_s(Entity->Name, Name);

    return Entity;
}

void RemoveEntity(game_entity_manager* EntityManager, int EntityID) {
    game_entity* Entity = &EntityManager->Entities.List[EntityID];
    Assert(Entity->Active);

    switch(Entity->Type) {
        case Entity_Type_Camera: {
            Remove(&EntityManager->Cameras, Entity->Index);
        } break;

        case Entity_Type_Character: {
            Remove(&EntityManager->Characters, Entity->Index);
        } break;

        case Entity_Type_Enemy: {
            Remove(&EntityManager->Enemies, Entity->Index);
        } break;

        case Entity_Type_Prop: {
            Remove(&EntityManager->Props, Entity->Index);
        } break;

        case Entity_Type_Weapon: {
            Remove(&EntityManager->Weapons, Entity->Index);
        } break;

        default: Raise("Invalid entity type.");
    }

    Remove(&EntityManager->Entities, Entity->ID);
}

void ClearEntities(game_entity_manager* EntityManager) {
    uint32 Index = 0;
    while (EntityManager->Cameras.Count > 1 && Index < MAX_CAMERAS) {
        camera* Camera = &EntityManager->Cameras.List[Index++];
        if (!Camera->OnAir && Camera->Entity) {
            RemoveEntity(EntityManager, Camera->Entity->ID);
        }
    }

    Index = 0;
    while (EntityManager->Entities.Count > 1)  {
        game_entity* Entity = &EntityManager->Entities.List[Index++];

        if (Entity->Type != Entity_Type_Camera && Entity->Active) {
            RemoveEntity(EntityManager, Entity->ID);
        }
    }

    EntityManager->Characters = {};
    EntityManager->Enemies = {};
    EntityManager->Props = {};
    EntityManager->Weapons = {};
}

game_entity* QueryEntity(game_entity_list* Entities, game_entity_type Type, bool Active = true) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        game_entity* Entity = &Entities->List[i];
        if (Entity->Type == Type && Entity->Active == Active) {
            return Entity;
        }
    }
    return 0;
}

int QueryEntityCount(game_entity_list* Entities, game_entity_type Type, bool Active = true) {
    int Result = 0;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        game_entity* Entity = &Entities->List[i];
        if (Entity->Type == Type && Entity->Active == Active) {
            Result++;
        }
    }
    return Result;
}

// Entity initialization ___________________________________________________________________________________________________________________

camera* AddCamera(
    game_entity_manager* EntityManager,
    v3 Position,
    float Angle, float Pitch,
    float Distance = 9.0
) {
    Assert(EntityManager->Cameras.Count < MAX_CAMERAS);

    camera* Cam = Insert(&EntityManager->Cameras);
    Cam->Angle = Angle;
    Cam->Pitch = Pitch;
    Cam->Position = Position;
    Cam->Distance = Distance;
    Cam->OnAir = EntityManager->Cameras.Count == 1;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Camera %d", Cam->ID);

    quaternion Rotation = Quaternion(Cam->Angle * Degrees, V3(0,1,0)) * Quaternion(Cam->Pitch * Degrees, V3(1,0,0));
    game_entity* Entity = AddEntity(&EntityManager->Entities, NameBuffer, Entity_Type_Camera, SphereCollider(Position, 1.0f), Position, Rotation, Scale(), Cam->ID == 0);
    Entity->Index = Cam->ID;
    Cam->Entity = Entity;

    return Cam;
}

weapon* AddWeapon(
    game_entity_manager* EntityManager,
    weapon_type Type,
    color Color = White,
    v3 Position = V3(0,0,0),
    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0),
    scale S = Scale()
) {
    Assert(EntityManager->Weapons.Count < MAX_WEAPONS);

    weapon* pWeapon = Insert(&EntityManager->Weapons);
    uint32 ID = pWeapon->ID;
    *pWeapon = WeaponTemplates[Type];
    pWeapon->ID = ID;
    pWeapon->Color = Color;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Weapon %d", pWeapon->ID);

    pWeapon->Entity = AddEntity(
        &EntityManager->Entities, 
        NameBuffer, 
        Entity_Type_Weapon,
        pWeapon->Collider,
        Position, 
        Rotation, 
        S
    );
    pWeapon->Entity->Index = pWeapon->ID;
    return pWeapon;
}

character* AddCharacter(game_entity_manager* EntityManager, character_class Class, v3 Position) {
    Assert(EntityManager->Characters.Count < MAX_CHARACTERS);

    character* pCharacter = Insert(&EntityManager->Characters);

    char NameBuffer[32];
    sprintf_s(NameBuffer, "%s %d", ClassNames[Class], pCharacter->ID);

    quaternion Rotation = Quaternion(1.5f * Pi, V3(0,1,0));
    pCharacter->Entity = AddEntity(
        &EntityManager->Entities, 
        NameBuffer, 
        Entity_Type_Character,
        CapsuleCollider(V3(0,0.6f,0), V3(0,4.0f,0), 0.8f),
        Position, 
        Rotation, 
        Scale()
    );
    pCharacter->Entity->Index = pCharacter->ID;

    pCharacter->Stats.MaxHP = 500;
    pCharacter->Stats.HP = pCharacter->Stats.MaxHP;

    switch (Class) {
        case Class_Knight: {
            weapon* Sword = AddWeapon(EntityManager, Weapon_Sword, White, V3(-5,0,0));
            weapon* Shield = AddWeapon(EntityManager, Weapon_Shield, White, V3(-10,0,0));
            Equip(Sword, pCharacter);
            Equip(Shield, pCharacter);

            pCharacter->Stats.Strength     = 15;
            pCharacter->Stats.Defense      = 12;
            pCharacter->Stats.Intelligence = 5;
            pCharacter->Stats.Wisdom       = 5;
            pCharacter->Stats.Speed        = 10;
            pCharacter->Stats.Precission   = 10;
        } break;

        case Class_Rogue: {
            weapon* Knife = AddWeapon(EntityManager, Weapon_Knife, White, V3(-5,0,0));
            Equip(Knife, pCharacter);

            pCharacter->Stats.Strength     = 8;
            pCharacter->Stats.Defense      = 8;
            pCharacter->Stats.Intelligence = 6;
            pCharacter->Stats.Wisdom       = 5;
            pCharacter->Stats.Speed        = 15;
            pCharacter->Stats.Precission   = 5;
        } break;
        
        case Class_Hunter: {
            weapon* Bow = AddWeapon(EntityManager, Weapon_Bow, White, V3(-5,0,0));
            Equip(Bow, pCharacter);

            pCharacter->Stats.Strength     = 10;
            pCharacter->Stats.Defense      = 6;
            pCharacter->Stats.Intelligence = 10;
            pCharacter->Stats.Wisdom       = 12;
            pCharacter->Stats.Speed        = 10;
            pCharacter->Stats.Precission   = 20;
        } break;

        case Class_Wizard: {
            weapon* Staff = AddWeapon(EntityManager, Weapon_Staff, White, V3(-5,0,0));
            Equip(Staff, pCharacter);

            pCharacter->Stats.Strength     = 5;
            pCharacter->Stats.Defense      = 6;
            pCharacter->Stats.Intelligence = 15;
            pCharacter->Stats.Wisdom       = 15;
            pCharacter->Stats.Speed        = 12;
            pCharacter->Stats.Precission   = 10;
        } break;
    }

    return pCharacter;
}

enemy* AddEnemy(game_entity_manager* EntityManager, v3 Position, enemy_type Type) {
    Assert(EntityManager->Characters.Count < MAX_ENEMIES);

    uint32 TypeID = EntityManager->nEnemyTypes[Type]++;

    // If any ID is free, use it
    enemy* pEnemy = Insert(&EntityManager->Enemies);
    FillTemplate(pEnemy, Type);

    char NameBuffer[32];
    sprintf_s(NameBuffer, "%s %d", EnemyNames[Type], TypeID);

    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0);
    collider Collider = EnemyColliders[Type];
    pEnemy->Entity = AddEntity(&EntityManager->Entities, NameBuffer, Entity_Type_Enemy, Collider, Position, Rotation, Scale());
    pEnemy->Entity->Index = pEnemy->ID;
    return pEnemy;
}

prop* AddProp(
    game_entity_manager* EntityManager, 
    game_mesh_id MeshID, 
    game_shader_pipeline_id Shader, 
    color Color = White,
    v3 Position = V3(0,0,0),
    quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0),
    scale S = Scale()
) {
    Assert(EntityManager->Props.Count < MAX_PROPS);
    // If any ID is free, use it
    int PropID = -1;
    if (EntityManager->Props.nFreeIDs > 0) {
        PropID = EntityManager->Props.FreeIDs[EntityManager->Props.nFreeIDs - 1];
        EntityManager->Props.FreeIDs[EntityManager->Props.nFreeIDs-- - 1] = -1;
        EntityManager->Props.Count++;
    }
    else PropID = EntityManager->Props.Count++;

    prop* pProp = &EntityManager->Props.List[PropID];
    pProp->MeshID = MeshID;
    pProp->Shader = Shader;
    pProp->Color = Color;

    char NameBuffer[32];
    sprintf_s(NameBuffer, "Prop %d", PropID);

    pProp->Entity = AddEntity(&EntityManager->Entities, NameBuffer, Entity_Type_Prop, SphereCollider(V3(0,0,0), 5.0f), Position, Rotation, S);
    pProp->Entity->Index = PropID;
    return pProp;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Combat                                                                                                                                       |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(altered_state,
    altered_state_none,
    altered_state_burnt,
    altered_state_wet,
    altered_state_frozen,
    altered_state_drowning,
    altered_state_poisoned,
    altered_state_rotting,
    altered_state_bleeding,
    altered_state_regenerating,
    altered_state_dead
);

ENUM(combatant_type,
    Combatant_Type_Player,
    Combatant_Type_Enemy
);

INTROSPECT
struct combatant {
    game_entity* Entity;
    stats* Stats;
    stats Modifier;
    magic_affinity Affinity;
    uint32 Index;
    float ATB;
    combatant_type Type;
    spell_id Spells[MAX_COMBATANT_SPELLS];
    bool AlteredState[altered_state_count];
};

const int MAX_COMBATANTS = 32;

combatant Combatant(character* Character) {
    combatant Result = {};
    Result.Stats = &Character->Stats;
    Result.Entity = Character->Entity;
    Result.ATB = 100.0f;
    Result.Type = Combatant_Type_Player;
    for (int i = 0; i < MAX_COMBATANT_SPELLS; i++) {
        Result.Spells[i] = Character->Spells[i];
    }
    return Result;
}

combatant Combatant(enemy* Enemy) {
    combatant Result = {};
    Result.Stats = &Enemy->Stats;
    Result.Entity = Enemy->Entity;
    Result.ATB = 100.0f;
    Result.Type = Combatant_Type_Enemy;
    for (int i = 0; i < MAX_COMBATANT_SPELLS; i++) {
        Result.Spells[i] = Enemy->Spells[i];
    }
    return Result;
}

ENUM(combatant_action,
    combatant_action_empty,
    combatant_action_attack,
    combatant_action_magic,
    combatant_action_items,
    combatant_action_flee
);

struct damage_animation {
    combatant* Combatant;
    uint32 ID;
    uint32 Damage;
    v2 Offset;
    float t;
    bool Active;
};

DefineFreeList(MAX_COMBATANTS, damage_animation);

void HealDamage(combatant* Target, uint32 Damage) {
    Target->Stats->HP += Damage;
    if (Target->Stats->HP > Target->Stats->MaxHP)
        Target->Stats->HP = Target->Stats->MaxHP;
}

void ApplyAlteredState(combatant* Target, altered_state State) {
    Target->AlteredState[State] = true;
}

void RemoveAlteredState(combatant* Target, altered_state State) {
    Target->AlteredState[State] = false;
}

void ApplyDamage(damage_animation_list* DamageAnimations, combatant* Target, uint32 Damage) {
    if  (Target->Stats->HP <= Damage) {
        Target->Stats->HP = 0;
        ApplyAlteredState(Target, altered_state_dead);
    }
    else Target->Stats->HP -= Damage;

    damage_animation* Animation = Insert(DamageAnimations);
    Animation->Combatant = Target;
    Animation->Active = true;
    Animation->Damage = Damage;
    Animation->t = 0;
    Animation->Offset = 30.0f * Radial(Tau * RandFloat());
}

void ApplyStatModifier(combatant* Target, stats Modifier) {
    Target->Modifier.MaxHP        += Modifier.MaxHP;
    Target->Modifier.Strength     += Modifier.Strength;
    Target->Modifier.Defense      += Modifier.Defense;
    Target->Modifier.Intelligence += Modifier.Intelligence;
    Target->Modifier.Wisdom       += Modifier.Wisdom;
    Target->Modifier.Speed        += Modifier.Speed;
    Target->Modifier.Precission   += Modifier.Precission;
}

void Update(damage_animation_list* CombatAnimations, render_group* Group, camera* Camera, float dt) {
    uint32 Index = 0;
    uint32 nAnimations = CombatAnimations->Count;
    const float Duration = 1.0f;
    // Represents the percentage of the animation duration that will be spent in a transition
    const float PercentageTransition = 0.25f; // This should be less than 0.5f (2 transitions: on and off)

    while (nAnimations > 0) {
        damage_animation* Animation = &CombatAnimations->List[Index];
        if (Animation->Active) nAnimations--;
        else {
            Index++;
            continue;
        }

        if (Animation->t > Duration) {
            Remove(CombatAnimations, Index);
        }
        else {
            Animation->t += dt;
            std::string Text = std::format("{}", Animation->Damage);
            v3 WorldPosition = Animation->Combatant->Entity->Transform.Translation;
            v2 ScreenPosition = GetScreenPosition(Group->Width, Group->Height, Camera, WorldPosition);
            ScreenPosition += Animation->Offset;

            float t = Animation->t / Duration;
            float Alpha = 1.0f;
            color Color = White;
            if (t < PercentageTransition) {
                Color = ChangeAlpha(Color, t / PercentageTransition);
                ScreenPosition += 10.0f * V2(0, 1.0f - t / PercentageTransition);
            }
            else if (t > 1.0f - PercentageTransition) {
                Color = ChangeAlpha(Color, (1.0f - t) / PercentageTransition);
                ScreenPosition -= 10.0f * V2(0, 1.0f - (1.0f - t) / PercentageTransition);
            }
            PushText(Group, ScreenPosition, Text.c_str(), .Color = Color);
        }

        Index++;
    }
}

struct turn {
    combatant* Attacker;
    uint32 Index;
    uint32 nTargets;
    combatant* Targets[MAX_COMBATANTS];
    float ATB[MAX_COMBATANTS];
    float ATBCost;
    combatant_action Action;
    spell_id Spell;
    item_type* UsedItem;
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
    uint32 nEnemies;
    uint32 nPlayers;
    bool Active;
};

void Erase(game_combat* Combat) {
    ClearArena(&Combat->TurnsArena);
    Clear(&Combat->Combatants);
    Combat->Turn = {};
    for (int i = 0; i < TURN_BUFFER_SIZE; i++) {
        Combat->NextTurns[i] = {};
    }
    Combat->nEnemies = 0;
    Combat->nPlayers = 0;
    Combat->DamageAnimations = {};
}

combatant* GetCombatant(combatant_array* Combatants, game_entity* Entity) {
    Assert(Entity != nullptr);

    for (int i = 0; i < Combatants->Count; i++) {
        combatant* Combatant = &Combatants->Content[i];
        if (Combatant->Entity == Entity) {
            return Combatant;
        }
    }

    return nullptr;
}

// Advances ATB of turn. If a new attacker is found, it is returned; returns NULL otherwise.
combatant* AdvanceTurnATB(combatant_array* Combatants, turn* Turn, int AttackerIndex = -1) {
    combatant* Result = NULL;
    float MaxSpeed = 0.0f;
    for (int i = 0; i < Combatants->Count; i++) {
        combatant* Combatant = &Combatants->Content[i];
        if (!Combatant->AlteredState[altered_state_dead]) {
            if (AttackerIndex != i) Turn->ATB[i] += Combatant->Stats->Speed;
            if (Turn->ATB[i] >= 100.0f) {
                if (
                    Combatant->Stats->Speed > MaxSpeed || 
                    // If current attacker's speed is equal to this potential attacker, flip a coin
                    Combatant->Stats->Speed == MaxSpeed && Bernoulli()
                ) Result = Combatant;
            }
            Turn->ATB[i] = Clamp(Turn->ATB[i], 0.0f, 100.0f);
        }
    }
    return Result;
}

// Applies ATB cost and advances turn ATB until new attacker is found.
turn GetNextTurn(combatant_array* Combatants, turn PreviousTurn) {
    turn Result = PreviousTurn;
    Result.Index++;
    Result.Attacker = NULL;
    Result.nTargets = 0;
    Result.Action = combatant_action_empty;
    Result.TargetsSelected = false;
    for (int i = 0; i < Combatants->Count; i++) {
        Result.Targets[i] = NULL;
    }

    // Apply ATB Cost
    Result.ATB[PreviousTurn.Attacker->Index] -= PreviousTurn.ATBCost;
    Result.Attacker = AdvanceTurnATB(Combatants, &Result, PreviousTurn.Attacker->Index);

    while (Result.Attacker == NULL) {
        Result.Attacker = AdvanceTurnATB(Combatants, &Result);
    }
    return Result;
}

void FillTurnBuffer(combatant_array* Combatants, turn Turn, turn* NextTurns) {
    NextTurns[0] = GetNextTurn(Combatants, Turn);
    for (int i = 1; i < TURN_BUFFER_SIZE; i++) {
        NextTurns[i] = GetNextTurn(Combatants, NextTurns[i-1]);
    }
}

void StartCombat(game_entity_manager* EntityManager, game_combat* Combat) {
    Erase(Combat);
    Combat->Active = true;

    // Add entities to struct and compute first attacker
    float MaxSpeed = 0.0f;
    uint32 nEntities = EntityManager->Entities.Count;
    uint32 Index = 0;
    while(nEntities > 0 && Index < MAX_ENTITIES) {
        game_entity* Entity = &EntityManager->Entities.List[Index++];
        if (!Entity->Active) continue;
        else nEntities--;
        
        combatant EntityCombatant;
        bool IsEnemy = Entity->Type == Entity_Type_Enemy;
        bool IsCharacter = Entity->Type == Entity_Type_Character;
        if (IsEnemy || IsCharacter) {
            if (IsEnemy) {
                enemy* Enemy = &EntityManager->Enemies.List[Entity->Index];
                EntityCombatant = Combatant(Enemy);\
                Combat->nEnemies++;
            }
            else if (IsCharacter) {
                character* Character = &EntityManager->Characters.List[Entity->Index];
                EntityCombatant = Combatant(Character);
                Combat->nPlayers++;
            }

            EntityCombatant.Index = Combat->Combatants.Count;
            combatant* Combatant = &Combat->Combatants.Content[EntityCombatant.Index];
            Append(&Combat->Combatants, EntityCombatant);
            Combat->Turn.ATB[Combatant->Index] = Combatant->ATB;

            // Compute first attacker
            if (EntityCombatant.Stats->Speed > MaxSpeed) {
                Combat->Turn.Attacker = Combatant;
                MaxSpeed = EntityCombatant.Stats->Speed;
            }
            else if (Combatant->Stats->Speed == MaxSpeed) {
                if (Bernoulli()) {
                    Combat->Turn.Attacker = Combatant;
                    MaxSpeed = EntityCombatant.Stats->Speed;
                }
            }
        }
    }

    Combat->Turn.Index = 0;
    Combat->Turn.ATBCost = 50.0f;
    Combat->Turn.nTargets = 1;
    Combat->Turn.TargetsSelected = false;

    FillTurnBuffer(&Combat->Combatants, Combat->Turn, Combat->NextTurns);
}

bool Apply(turn Turn, damage_animation_list* DamageAnimations) {
    bool UpdateTurnBuffer = false;
    for (int i = 0; i < Turn.nTargets; i++) {
        combatant* Target = Turn.Targets[i];
        switch (Turn.Action) {
            case combatant_action_attack: {
                uint32 Strength = Turn.Attacker->Stats->Strength + Turn.Attacker->Modifier.Strength;
                uint32 Precission = Turn.Attacker->Stats->Precission + Turn.Attacker->Modifier.Precission;

                float Mu = 2.0f * (float)Strength;
                float Sigma = Mu / (float)Precission;

                uint32 Damage = CustomRound(Normal(Mu, Sigma));

                ApplyDamage(DamageAnimations, Target, Damage);
            } break;

            case combatant_action_magic: {
                spell Spell = Spells[Turn.Spell];

                uint32 Damage = Spell.Damage + Turn.Attacker->Stats->Intelligence;

                // Altered states
                switch (Turn.Spell) {
                    case Spell_Fireball:
                    case Spell_Burn:
                    case Spell_Incinerate:
                    case Spell_Explosion: {
                        if (Bernoulli(0.4f)) {
                            ApplyAlteredState(Target, altered_state_burnt);
                        }
                        if (Target->AlteredState[altered_state_frozen]) {
                            RemoveAlteredState(Target, altered_state_frozen);
                        }
                        ApplyDamage(DamageAnimations, Target, Damage);
                    } break;

                    case Spell_Drown:
                        ApplyAlteredState(Target, altered_state_drowning);
                    case Spell_Drench:
                    case Spell_Wave:
                    case Spell_Cascade:
                    case Spell_Wash:
                    case Spell_Hydrate: {
                        ApplyAlteredState(Target, altered_state_wet);
                        ApplyDamage(DamageAnimations, Target, Damage);
                    } break;

                    case Spell_Icicle:
                    case Spell_Blizzard:
                    case Spell_Freeze:
                    case Spell_Absolute_Zero: {
                        ApplyAlteredState(Target, altered_state_frozen);
                        ApplyDamage(DamageAnimations, Target, Damage);
                    } break;
                        
                    // Air (Octahedron)
                        // Spell_Wind,
                        // Spell_Gust,
                        // Spell_Fly,
                        // Spell_Air_Shield,
                        // Spell_Tornado,
                        // Spell_Hurricane,

                    case Spell_Poison: {
                        ApplyAlteredState(Target, altered_state_poisoned);
                    } break;

                    case Spell_Rot: {
                        ApplyAlteredState(Target, altered_state_rotting);
                    } break;

                    case Spell_Bleed: {
                        ApplyAlteredState(Target, altered_state_bleeding);
                    } break;

                    case Spell_Kill:
                    case Spell_Multikill: {
                        if (Bernoulli(0.1f)) {
                            ApplyAlteredState(Target, altered_state_dead);
                        }
                        else {
                            ApplyDamage(DamageAnimations, Target, Spell.Damage);
                        }
                    }

                    case Spell_Cure: 
                    case Spell_Multicure: {
                        for (int j = 0; j < altered_state_count; j++) {
                            RemoveAlteredState(Target, (altered_state)j);
                        }
                    } break;

                    case Spell_Heal: {
                        HealDamage(Target, Damage);
                    } break;

                    case Spell_Regeneration: 
                    case Spell_Multiheal: {
                        ApplyAlteredState(Target, altered_state_regenerating);
                    } break;

                    case Spell_Resurrect: {
                        RemoveAlteredState(Target, altered_state_dead);
                    } break;

                    case Spell_Slow: {
                        stats Modifier = {};
                        Modifier.Speed /= 2;
                        ApplyStatModifier(Target, Modifier);
                    } break;

                    case Spell_Accelerate: {
                        stats Modifier = {};
                        Modifier.Speed *= 2;
                        ApplyStatModifier(Target, Modifier);
                    } break;

                    case Spell_Stop: {
                        ApplyAlteredState(Target, altered_state_frozen);
                    } break;

                    default: {
                        Raise("NOT IMPLEMENTED");
                    }
                }
            } break;

            case combatant_action_items: {
                switch (*Turn.UsedItem) {
                    case Item_Type_Potion: {
                        HealDamage(Target, 50);
                    } break;

                    case Item_Type_Antidote: {
                        RemoveAlteredState(Target, altered_state_poisoned);
                    } break;

                    case Item_Type_Bomb: {
                        ApplyDamage(DamageAnimations, Target, 50);
                    } break;

                    case Item_Type_Phoenix_Feather: {
                        RemoveAlteredState(Target, altered_state_dead);
                    } break;

                    case Item_Type_Lightning_Bottle: {
                        ApplyDamage(DamageAnimations, Target, 75);
                    } break;

                    case Item_Type_Water_Bottle: {
                        ApplyAlteredState(Target, altered_state_wet);
                    } break;

                    case Item_Type_Poison: {
                        ApplyAlteredState(Target, altered_state_poisoned);
                    } break;

                    case Item_Type_Acid: {
                        ApplyAlteredState(Target, altered_state_burnt);
                    } break;
                }
                *Turn.UsedItem = Item_Type_None;
            } break;

            default: Raise("Invalid or empty combatant action.");
        }

        // Did someone die?
        if (Target->AlteredState[altered_state_dead]) {
            UpdateTurnBuffer = true;
        }
    }

    if (
        Turn.Attacker->AlteredState[altered_state_burnt] || 
        Turn.Attacker->AlteredState[altered_state_drowning] ||
        Turn.Attacker->AlteredState[altered_state_poisoned] ||
        Turn.Attacker->AlteredState[altered_state_rotting] ||
        Turn.Attacker->AlteredState[altered_state_bleeding]
    ) {
        ApplyDamage(DamageAnimations, Turn.Attacker, 10);
    }

    return UpdateTurnBuffer;
}

void EndTurn(
    damage_animation_list* DamageAnimations, 
    memory_arena* TurnsArena, 
    combatant_array* Combatants, 
    turn* Turn, 
    turn* NextTurns
) {
    bool UpdateTurnBuffer = Apply(*Turn, DamageAnimations);

    if (UpdateTurnBuffer) {
        FillTurnBuffer(Combatants, *Turn, NextTurns);
    }

    turn* History = PushStruct(TurnsArena, turn);
    *History = *Turn;

    *Turn = NextTurns[0];
    for (int i = 1; i < TURN_BUFFER_SIZE; i++) {
        NextTurns[i-1] = NextTurns[i];
    }
    turn LastKnown = NextTurns[TURN_BUFFER_SIZE - 1];
    NextTurns[TURN_BUFFER_SIZE - 1] = GetNextTurn(Combatants, LastKnown);
}

void EndCombat(game_combat* Combat, game_entity_manager* EntityManager, uint32* Gold, bool Success = true) {
    Combat->Active = false;

    for (int i = 0; i < Combat->Combatants.Count; i++) {
        combatant* Combatant = &Combat->Combatants.Content[i];
        if (Combatant->Type == Combatant_Type_Enemy) {
            RemoveEntity(EntityManager, Combatant->Entity->ID);
        }
    }
    Erase(Combat);

    for (int i = 0; i < enemy_type_count; i++) {
        EntityManager->nEnemyTypes[i] = 0;
    }

    if (Success) {
        *Gold += 10;
    }
    else {
        *Gold = 0;
    }
}

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

void RandomizeLevel(level* Level, room_type FirstRoomType) {
    *Level = {};
    Level->nRooms = 0;

    room* FirstRoom = AddRoom(Level, FirstRoomType);

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
    game_entity_manager EntityManager;
    particle_emitter* Emitter;
    game_combat Combat;
    level Level;
    uint32 Gold;
    item_type Store[3];
    item_type Inventory[3];
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
    game_entity_manager* EntityManager = &State->EntityManager;
    
    State->Type = Type;

    switch(Type) {
        case Game_State_Combat: {
            if (State->CurrentRoom->Type == Room_Type_Combat || State->CurrentRoom->Type == Room_Type_Quest) {
                uint32 nEnemies = RandInt(2, 4);
                v3 Position = V3(10, 0, -5 * ((nEnemies - 1) / 2.0f));

                for (int i = 0; i < nEnemies; i++) {
                    enemy_type EnemyType = (enemy_type)RandInt(0, EnemyTypeFirstMiniboss);
                    AddEnemy(&State->EntityManager, Position, EnemyType);
                    Position.Z += 5.0f;
                }
            }
            else if (State->CurrentRoom->Type == Room_Type_Miniboss) {
                enemy_type EnemyType = (enemy_type)RandInt(EnemyTypeFirstMiniboss, EnemyTypeFirstBoss);
                AddEnemy(&State->EntityManager, V3(10, 0, 0), EnemyType);
            }
            else if (State->CurrentRoom->Type == Room_Type_Boss) {
                enemy_type EnemyType = (enemy_type)RandInt(EnemyTypeFirstBoss, enemy_type_count);
                AddEnemy(&State->EntityManager, V3(10, 0, 0), EnemyType);
            }
            else Raise("Invalid room type for game state combat.");

            if (State->CurrentRoom->Type == Room_Type_Quest) {
                character_class Companion = RandomEnum(character_class);

                AddCharacter(&State->EntityManager, Companion, V3(0,0,0));
            }

            StartCombat(&State->EntityManager, &State->Combat);
        } break;

        case Game_State_Trade: {
            State->Store[0] = RandomItemType();
            State->Store[1] = RandomItemType();
            while (State->Store[1] == State->Store[0]) {
                State->Store[1] = RandomItemType();
            }
            State->Store[2] = RandomItemType();
            while (State->Store[2] == State->Store[0] || State->Store[2] == State->Store[1]) {
                State->Store[2] = RandomItemType();
            }
        } break;

        case Game_State_Camp: {
            uint32 nCharacters = EntityManager->Characters.Count;
            uint32 Index = 0;
            while (nCharacters > 0) {
                character* Character = &EntityManager->Characters.List[Index++];

                if (Character->Entity != NULL) nCharacters--;
                else continue;

                Character->Stats.HP = Character->Stats.MaxHP;
            }

            State->CampTime = 0;
        } break;
    }
}

void UpdateEntities(render_group* Group, game_state* State, game_input* Input) {
    game_entity_manager* EntityManager = &State->EntityManager;
    game_combat* Combat = &State->Combat;
    uint32 Index = 0;

// Combat
    if (Combat->Active) {
        combatant* Hot = NULL;
        combatant_array* Combatants = &Combat->Combatants;
        for (int i = 0; i < Combatants->Count; i++) {
            combatant* Combatant = &Combatants->Content[i];
            if (!Combatant->AlteredState[altered_state_dead] && Combatant->Entity->Hovered) {
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
                        Turn->TargetsSelected = true;
                    }
                } break;

                case combatant_action_magic: {
                    if (Hot->Type != Turn->Attacker->Type && Turn->Spell != Spell_Empty) {
                        Turn->nTargets = 1;
                        Turn->Targets[0] = Hot;
                        Turn->TargetsSelected = true;
                    }
                } break;

                case combatant_action_items: {
                    Turn->nTargets = 1;
                    Turn->Targets[0] = Hot;
                    Turn->TargetsSelected = true;
                } break;
            }
        }

        if (Combat->Turn.TargetsSelected) {
            EndTurn(
                &Combat->DamageAnimations, 
                &Combat->TurnsArena, 
                &Combat->Combatants, 
                &Combat->Turn,
                Combat->NextTurns
            );
        }

        Update(&Combat->DamageAnimations, Group, State->ActiveCamera, State->dt);

        bool SomePlayerAlive = false;
        bool AllEnemiesDead = true;
        for (int i = 0; i < Combatants->Count; i++) {
            combatant* Combatant = &Combatants->Content[i];
            if (Combatant->Type == Combatant_Type_Player ) {
                SomePlayerAlive |= !Combatant->AlteredState[altered_state_dead];
            }
            else if (Combatant->Type == Combatant_Type_Enemy) {
                AllEnemiesDead &= Combatant->AlteredState[altered_state_dead];
            }
            if (SomePlayerAlive && !AllEnemiesDead) break;
        }
        if (!SomePlayerAlive || AllEnemiesDead) {
            EndCombat(Combat, &State->EntityManager, &State->Gold, AllEnemiesDead);

            if (SomePlayerAlive) {
                // Advance room
                if (State->CurrentRoom->Type == Room_Type_Boss) {
                    room_type FirstRoomType = (room_type)RandInt(0, Room_Type_Miniboss);
                    RandomizeLevel(&State->Level, FirstRoomType);
                    State->CurrentRoom = &State->Level.Rooms[0];
                    Transition(State, GetStateType(FirstRoomType));
                }
                else {
                    Transition(State, Game_State_Map);
                }
            }
        }
    }

// Cameras _________________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nCameras = EntityManager->Cameras.Count;
    while(nCameras > 0) {
        camera* Cam = &EntityManager->Cameras.List[Index++];
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
        Cam->View = GetViewMatrix(Cam);

        break;
    }
    
// Characters ______________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nCharacters = EntityManager->Characters.Count;
    while (nCharacters > 0) {
        character* Character = &EntityManager->Characters.List[Index++];

        if (Character->Entity != NULL) nCharacters--;
        else continue;

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
    uint32 nEnemies = EntityManager->Enemies.Count;
    while (nEnemies > 0) {
        enemy* pEnemy = &EntityManager->Enemies.List[Index++];
        if (pEnemy->Entity != NULL) nEnemies--;
        else continue;

        if (pEnemy->Type == Enemy_Type_Horns) {
            if (pEnemy->Stats.HP > 0) {
                pEnemy->Entity->Transform.Translation.Y = 3.2 + sin(3 * State->Time);
            }
            else {
                pEnemy->Entity->Transform.Translation.Y = 0;
            }
        }

        v3 FacingDirection = V3(-1,0,0);
        if (State->ControlledCharacter != NULL && State->ControlledCharacter->Entity != NULL) {
            FacingDirection = State->ControlledCharacter->Entity->Transform.Translation - pEnemy->Entity->Transform.Translation;
        }
        float Angle = atan2f(FacingDirection.Z, FacingDirection.X);
        pEnemy->Entity->Transform.Rotation = Quaternion(Angle, V3(0,1,0));

        // Combat AI
        if (Combat->Active && Combat->Turn.Attacker->Entity == pEnemy->Entity) {
            uint32 CheckedPlayers = 0;
            for (int i = 0; i < Combat->Combatants.Count; i++) {
                combatant* Combatant = &Combat->Combatants.Content[i];
                if (Combatant->Type == Combatant_Type_Player) {
                    if (CheckedPlayers == Combat->nPlayers - 1 || Bernoulli()) {
                        Combat->Turn.Action = combatant_action_attack;
                        Combat->Turn.nTargets = 1;
                        Combat->Turn.Targets[0] = Combatant;
                        Combat->Turn.TargetsSelected = true;
                        break;
                    }
                }
            }
        }
    }

// Weapons _________________________________________________________________________________________________________________________________
    Index = 0;
    uint32 nWeapons = EntityManager->Weapons.Count;
    while (nWeapons > 0) {
        weapon* pWeapon = &EntityManager->Weapons.List[Index++];
        if (pWeapon->Entity != NULL) nWeapons--;
        else continue;

        pWeapon->Entity->Collided = false;

        if (pWeapon->ParentBone == -1) {
            pWeapon->Entity->Transform.Rotation = Quaternion(State->Time, V3(0,1,0));
        }
        else {
            character* Owner = &EntityManager->Characters.List[pWeapon->Entity->Parent->Index];
            bone Bone = Owner->Armature.Bones[pWeapon->ParentBone];
            pWeapon->Entity->Transform = pWeapon->Transform * Bone.Transform * Owner->Entity->Transform;
        }
    }
}

void PushEntities(render_group* Group, camera* Camera, game_state* GameState, game_input* Input, float Time) {
    game_combat* Combat = &GameState->Combat;
    game_entity_manager* EntityManager = &GameState->EntityManager;
    game_assets* Assets = Group->Assets;

    basis Basis = Camera->Basis;
    ray Ray = MouseRay(Group->Width, Group->Height, Camera->Position + Camera->Distance * Basis.Z, Basis, Input->Mouse.Cursor);
    int i = 0;
    int nEntities = EntityManager->Entities.Count;
    while (nEntities > 0 && i < MAX_ENTITIES) {
        game_entity* Entity = &EntityManager->Entities.List[i++];

        if (Entity->Active) nEntities--;
        else continue;

        collider Collider = Entity->Transform * Entity->Collider;
        Entity->Hovered = Raycast(Ray, Collider);
        bool Outline = Entity->Hovered && (
            Combat->Turn.Action == combatant_action_attack ||
            Combat->Turn.Action == combatant_action_magic && Combat->Turn.Spell != Spell_Empty ||
            Combat->Turn.Action == combatant_action_items && Combat->Turn.UsedItem && *Combat->Turn.UsedItem != Item_Type_None
        );
        switch(Entity->Type) {
            case Entity_Type_Character: {
                character* pCharacter = &EntityManager->Characters.List[Entity->Index];
                game_mesh* Mesh = GetAsset(Assets, Mesh_Body_ID);
                color Color = White;

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

                    combatant* Combatant = GetCombatant(&Combat->Combatants, Entity);
                    if (Combatant) {
                        if (Combatant->AlteredState[altered_state_burnt]) {
                            Color = Orange;
                        }
                        else if (Combatant->AlteredState[altered_state_drowning]) {
                            Color = Blue;
                        }
                        else if (Combatant->AlteredState[altered_state_poisoned]) {
                            Color = Green;
                        }
                        else if (Combatant->AlteredState[altered_state_rotting]) {
                            Color = Purple;
                        }
                        else if (Combatant->AlteredState[altered_state_bleeding]) {
                            Color = Red;
                        }
                    }
                }

                PushMesh(
                    Group,
                    Mesh_Body_ID,
                    Entity->Transform,
                    Shader_Pipeline_Mesh_Bones_ID,
                    Bitmap_Empty_ID,
                    Color,
                    &pCharacter->Armature,
                    Outline
                );
            } break;
    
            case Entity_Type_Enemy: {
                enemy* pEnemy = &EntityManager->Enemies.List[Entity->Index];
                game_mesh* Mesh = GetAsset(Assets, pEnemy->MeshID);
                color Color = White;

                transform DeadTransform = IdentityTransform;
                if (pEnemy->Stats.HP == 0) {
                    DeadTransform.Rotation = Quaternion(90 * Degrees, V3(1,0,0));
                }

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

                    combatant* Combatant = GetCombatant(&Combat->Combatants, Entity);
                    if (Combatant) {
                        if (Combatant->AlteredState[altered_state_burnt]) {
                            Color = Orange;
                        }
                        else if (Combatant->AlteredState[altered_state_drowning]) {
                            Color = Blue;
                        }
                        else if (Combatant->AlteredState[altered_state_poisoned]) {
                            Color = Green;
                        }
                        else if (Combatant->AlteredState[altered_state_rotting]) {
                            Color = Purple;
                        }
                        else if (Combatant->AlteredState[altered_state_bleeding]) {
                            Color = Red;
                        }
                    }
                }

                PushMesh(
                    Group,
                    pEnemy->MeshID,
                    DeadTransform * Entity->Transform,
                    Shader_Pipeline_Mesh_ID,
                    pEnemy->TextureID,
                    Color, nullptr,
                    Outline
                );
            } break;

            case Entity_Type_Prop: {
                prop* pProp = &EntityManager->Props.List[Entity->Index];
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
                weapon* pWeapon = &EntityManager->Weapons.List[Entity->Index];
                PushMesh(Group, pWeapon->MeshID, Entity->Transform, Shader_Pipeline_Mesh_ID);
            } break;
        }

        if (Group->Debug && Group->DebugColliders && Entity->Type != Entity_Type_Camera) {
            PushCollider(Group, Entity->Collider, Entity->Transform, Entity->Collided ? Red : Yellow);
        }
    }
}

#endif