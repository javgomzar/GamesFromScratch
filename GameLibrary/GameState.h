#ifndef GAME_ENTITY
#define GAME_ENTITY

#include "GameInput.h"
#include "GameRender.h"
#include "Particles.h"

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Entities                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(game_entity_type,
    Character_Entity_Type,
    Enemy_Entity_Type,
    Camera_Entity_Type,
    Prop_Entity_Type,
    Weapon_Entity_Type
);

FLAGS(entity_flag,
    render,
    movable,
    animated
);

INTROSPECT
struct game_entity {
    char Name[32];
    uint32 Index;
    entity_flag Flags = entity_flag::none;
    game_entity_type Type;
    game_entity* Parent = nullptr;
    game_entity* Follow = nullptr;
    collider Collider = SphereCollider(V3(0,0,0), 1.0f);
    color Color = White;

    transform Transform = IdentityTransform;
    v3 Velocity = V3(0,0,0);
    v3 AngularVelocity = V3(0,0,0);

    // Rendering
    game_mesh_id MeshID = Mesh_Sphere_ID;
    game_bitmap_id TextureID = Bitmap_Empty_ID;

    // View
    v3 Anchor = V3(0,0,0);
    float Distance = 0.0f;
    float Angle = 0.0f;
    float Pitch = 0.0f;

    // Combat
    game_entity* LeftHand = nullptr;
    game_entity* RightHand = nullptr;

    bool Collided = false;
    bool Active = false;
    bool Hovered = false;
};

uint32 EntitySize = sizeof(game_entity);

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

/*
    Get vector basis for a camera given its angle and pitch. Relative to the camera:
        - The X axis is left to right.
        - The Y axis is down to up.
        - The Z axis is back to front.
    Note that this basis is left-handed.
*/
basis GetCameraBasis(float Angle, float Pitch) {
    float cosA = cosf(Angle * Degrees);
    float sinA = sinf(Angle * Degrees);
    float cosP = cosf(Pitch * Degrees);
    float sinP = sinf(Pitch * Degrees);

    basis Result;
    Result.X = V3(        cosA,  0.0,        -sinA);
    Result.Y = V3(-sinA * sinP, cosP, -cosA * sinP);
    Result.Z = V3(-sinA * cosP,-sinP, -cosA * cosP);
    return Result;
}

matrix4 GetViewMatrix(matrix3 Basis, float Distance, v3 Position) {
	Basis = transpose(Basis);

	v3 Translation = V3(0, 0, Distance) - Position * Basis;
	matrix4 Result;
	Result.X = V4(Basis.X, 0);
	Result.Y = V4(Basis.Y, 0);
	Result.Z = V4(Basis.Z, 0);
	Result.W = V4(Translation, 1);

	return Result;
}

v2 GetScreenPosition(float ScreenWidth, float ScreenHeight, matrix4 View, v3 WorldPosition) {
    v4 Point = V4(WorldPosition, 1);
    matrix4 Projection = GetWorldProjectionMatrix(ScreenWidth, ScreenHeight);
    v4 ViewPoint = Point * View * Projection;
    float Factor = ViewPoint.W == 0 ? 0 : 1.0f/ViewPoint.W;
    v2 DevicePoint = Factor * V2(ViewPoint.X, ViewPoint.Y);
    v2 ScreenPoint = V2(
        0.5f * (1.0f + DevicePoint.X) * ScreenWidth,
        0.5f * (1.0f - DevicePoint.Y) * ScreenHeight
    );
    return ScreenPoint;
}

ray MouseRay(float Width, float Height, v3 CameraPosition, basis CameraBasis, v2 Mouse) {
	v3 Direction =
        (2.0 * Mouse.X / Width - 1.0) *    CameraBasis.X +
        (Height - 2.0 * Mouse.Y) / Width * CameraBasis.Y + 
                                           CameraBasis.Z;
	ray Result = Ray(CameraPosition, Direction);
	return Result;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Weapons                                                                                                                                      |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

transform WeaponTransforms[] = {
    GetTransform(
        V3(0.5f,2.0f,0),
        Quaternion(-0.25f * Tau, V3(0,1,0)) * Quaternion(-0.25f * Tau, V3(1,0,0))
    ),
    GetTransform(
        V3(-0.7f,2.2f,0),
        Quaternion(0.5f * Tau, V3(0,0,1)) * Quaternion(0.25f * Tau, V3(1,0,0))
    ),
};

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

character_action GetCharacterAction(character_action CurrentAction, game_animator* Animator, game_input* Input) {
    bool JumpingInput = Input->Mode == Keyboard && Input->Keyboard.Space.JustPressed ||
                        Input->Mode == Controller && Input->Controller.BButton.JustPressed;

    bool AttackInput = Input->Mode == Keyboard && Input->Keyboard.E.JustPressed ||
                       Input->Mode == Controller && Input->Controller.XButton.JustPressed;

    bool KeyboardMoving = Input->Keyboard.W.IsDown != Input->Keyboard.S.IsDown ||
                          Input->Keyboard.A.IsDown != Input->Keyboard.D.IsDown;
    bool ControllerMoving = fabs(Input->Controller.LeftJoystick.X) > 0.1 || fabs(Input->Controller.LeftJoystick.Y) > 0.1;
    bool MovingInput = (Input->Mode == Keyboard && KeyboardMoving) ||
                       (Input->Mode == Controller && ControllerMoving);

    character_action Result = CurrentAction;

    switch(CurrentAction.ID) {
        case Character_Action_Idle_ID: {
            Animator->Active = true;
            if (JumpingInput || MovingInput || AttackInput) {
                Animator->CurrentFrame = 0;
            }

            if     (JumpingInput) Result = CharacterAction(Character_Action_Jump_ID);
            else if (MovingInput) Result = CharacterAction(Character_Action_Walk_ID);
            else if (AttackInput) Result = CharacterAction(Character_Action_Attack_ID);
        } break;
        case Character_Action_Walk_ID: {
            if (JumpingInput) {
                Result = CharacterAction(Character_Action_Jump_ID);
                Animator->CurrentFrame = 0;
            }
            else if (AttackInput) {
                Result = CharacterAction(Character_Action_Attack_ID);
                Animator->CurrentFrame = 0;
            }
            else if (!MovingInput) {
                Animator->Active = false;
                Result = CharacterAction(Character_Action_Idle_ID);
            }
        } break;
        case Character_Action_Jump_ID: {
            if (!Animator->Active) {
                Result = CharacterAction(Character_Action_Idle_ID);
                Animator->CurrentFrame = 0;
            }
        } break;
        case Character_Action_Attack_ID: {
            if (!Animator->Active) {
                Result = CharacterAction(Character_Action_Idle_ID);
                Animator->CurrentFrame = 0;
            }
        } break;
        default: Raise("Invalid character action");
    }

    Animator->Loop = Result.Loop;
    return Result;
}
/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Game state                                                                                                                                   |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/
const int MAX_ENTITIES = 128;

struct game_state {
    free_list<game_entity> Entities;
    particle_emitter* Emitter;
    game_entity* ActiveCamera;
    double dt;
    float Time;
    bool Exit;
};

game_entity* CreateEntity(free_list<game_entity>& Entities, const char* Name, game_entity_type Type, bool Active = true) {
    game_entity Entity;

    strcpy(Entity.Name, Name);
    Entity.Type = Type;
    Entity.Active = Active;
    switch(Type) {
        case Camera_Entity_Type: {
            Entity.Angle = 0;
            Entity.Pitch = 0;
            Entity.Distance = 10.0f;
        } break;
        case Character_Entity_Type: {
            Entity.MeshID = Mesh_Body_ID;
            Entity.Flags = entity_flag::render | entity_flag::movable | entity_flag::animated;
        } break;
        case Prop_Entity_Type: {
            Entity.Flags = entity_flag::render;
        } break;
        case Weapon_Entity_Type: {
            Entity.Flags = entity_flag::render;
        } break;
        default: {
            Raise("Invalid entity type.");
        }
    }

    uint32 Index = Entities.Insert(Entity);
    game_entity* Result = &Entities[Index];
    Result->Index = Index;
    return Result;
}

#endif