#ifndef GAME_ENTITY
#define GAME_ENTITY

#include "GameInput.h"
#include "GameRender.h"
#include "Particles.h"

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Entities                                                                                                                                     |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/

ENUM(Entity_type,
    Character_Entity_Type,
    Enemy_Entity_Type,
    Camera_Entity_Type,
    Prop_Entity_Type,
    Weapon_Entity_Type
);

FLAGS(Entity_Flag,
    Render,
    Movable,
    Animated
);

struct Entity {
    String name;
    Entity_Flag flags = Entity_Flag::none;
    Entity_type type;
    Entity* parent = nullptr;
    Entity* follow = nullptr;
    Collider collider = SphereCollider(V3(0,0,0), 1.0f);
    Color color = Color::WHITE;

    Transform transform = IdentityTransform;
    V3 velocity = V3(0,0,0);
    V3 angular_velocity = V3(0,0,0);

    // Rendering
    Mesh* mesh = nullptr;
    Texture* texture = nullptr;

    // View
    V3 anchor = V3(0,0,0);
    float distance = 0.0f;
    float angle = 0.0f;
    float pitch = 0.0f;

    // Combat
    Entity* left_hand = nullptr;
    Entity* right_hand = nullptr;

    bool collided = false;
    bool active = false;
    bool hovered = false;
};

uint32 EntitySize = sizeof(Entity);

bool Collide(Entity* entity1, Entity* entity2) {
    Collider collider1 = entity1->collider;
    collider1.offset += entity1->transform.translation;
    if (collider1.type == Capsule_Collider) {
        collider1.capsule.segment = entity1->transform * collider1.capsule.segment;
    }
    Collider Collider2 = entity2->collider;
    Collider2.offset += entity2->transform.translation;
    if (Collider2.type == Capsule_Collider) {
        Collider2.capsule.segment = entity2->transform * Collider2.capsule.segment;
    }
    if (Collide(collider1, Collider2)) {
        entity1->collided = true;
        entity2->collided = true;
        return true;
    }
    return false;
}

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Camera                                                                                                                                       |
+----------------------------------------------------------------------------------------------------------------------------------------------+s
*/

/*
    Get vector basis for a camera given its angle and pitch. Relative to the camera:
        - The X axis is left to right.
        - The Y axis is down to up.
        - The Z axis is back to front.
    Note that this basis is left-handed.
*/
Basis GetCameraBasis(float Angle, float Pitch) {
    float cosA = cosf(Angle * DEGREES);
    float sinA = sinf(Angle * DEGREES);
    float cosP = cosf(Pitch * DEGREES);
    float sinP = sinf(Pitch * DEGREES);

    Basis Result;
    Result.x = V3(        cosA,  0.0,        -sinA);
    Result.y = V3(-sinA * sinP, cosP, -cosA * sinP);
    Result.z = V3(-sinA * cosP,-sinP, -cosA * cosP);
    return Result;
}

Matrix4 GetViewMatrix(Matrix3 basis, float distance, V3 position) {
	basis = transpose(basis);

	V3 translation = V3(0, 0, distance) - position * basis;
	Matrix4 result;
	result.x = BuildV4(basis.x, 0);
	result.y = BuildV4(basis.y, 0);
	result.z = BuildV4(basis.z, 0);
	result.w = BuildV4(translation, 1);

	return result;
}

V2 GetScreenPosition(float ScreenWidth, float ScreenHeight, Matrix4 View, V3 WorldPosition) {
    V4 Point = BuildV4(WorldPosition, 1);
    Matrix4 Projection = GetWorldProjectionMatrix(ScreenWidth, ScreenHeight);
    V4 ViewPoint = Point * View * Projection;
    float Factor = ViewPoint.w == 0 ? 0 : 1.0f/ViewPoint.w;
    V2 DevicePoint = Factor * V2(ViewPoint.x, ViewPoint.y);
    V2 ScreenPoint = V2(
        0.5f * (1.0f + DevicePoint.x) * ScreenWidth,
        0.5f * (1.0f - DevicePoint.y) * ScreenHeight
    );
    return ScreenPoint;
}

Ray MouseRay(float Width, float Height, V3 CameraPosition, Basis CameraBasis, V2 Mouse) {
	V3 Direction =
        (2.0 * Mouse.x / Width - 1.0) *    CameraBasis.x +
        (Height - 2.0 * Mouse.y) / Width * CameraBasis.y + 
                                           CameraBasis.z;
	Ray Result = BuildRay(CameraPosition, Direction);
	return Result;
}

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Weapons                                                                                                                                      |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/

Transform WeaponTransforms[] = {
    GetTransform(
        BuildV3(0.5f,2.0f,0),
        BuildQuaternion(-0.25f * TAU, BuildV3(0,1,0)) * BuildQuaternion(-0.25f * TAU, V3(1,0,0))
    ),
    GetTransform(
        BuildV3(-0.7f,2.2f,0),
        BuildQuaternion(0.5f * TAU, BuildV3(0,0,1)) * BuildQuaternion(0.25f * TAU, V3(1,0,0))
    ),
};

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Character                                                                                                                                    |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/

ENUM(character_action_id,
    Character_Action_Idle_ID,
    Character_Action_Walk_ID,
    Character_Action_Jump_ID,
    Character_Action_Attack_ID
);

struct character_action {
    character_action_id ID;
    Animation* animation;
    bool Loop;
};

// character_action GetCharacterAction(Asset_Manager* manager, character_action_id ID) {
//     character_action Result = {};
//     Result.ID = ID;

//     switch(ID) {
//         case Character_Action_Idle_ID: {
//             Result.animation = GetAnimation(manager, "Idle");
//             Result.Loop = true;
//         } break;
//         case Character_Action_Walk_ID: {
//             Result.animation = GetAnimation(manager, "Walk");
//             Result.Loop = true;
//         } break;
//         case Character_Action_Jump_ID: {
//             Result.animation = GetAnimation(manager, "Jump");
//         } break;
//         case Character_Action_Attack_ID: {
//             Result.animation = GetAnimation(manager, "Attack");
//         } break;
//         default: Assert(false);
//     }

//     return Result;
// }

// character_action GetCharacterAction(
//     character_action CurrentAction, 
//     Asset_Manager* manager,
//     Animator* animator, 
//     Input* input
// ) {
//     bool JumpingInput = input->mode == Keyboard && input->keyboard.space.just_pressed ||
//                         input->mode == Controller && input->controller.B_button.just_pressed;

//     bool AttackInput = input->mode == Keyboard && input->keyboard.E.just_pressed ||
//                        input->mode == Controller && input->controller.X_button.just_pressed;

//     bool KeyboardMoving = input->keyboard.W.is_down != input->keyboard.S.is_down ||
//                           input->keyboard.A.is_down != input->keyboard.D.is_down;
//     bool ControllerMoving = fabs(input->controller.left_joystick.x) > 0.1 || fabs(input->controller.left_joystick.y) > 0.1;
//     bool MovingInput = (input->mode == Keyboard && KeyboardMoving) ||
//                        (input->mode == Controller && ControllerMoving);

//     character_action Result = CurrentAction;

//     switch(CurrentAction.ID) {
//         case Character_Action_Idle_ID: {
//             animator->Active = true;
//             if (JumpingInput || MovingInput || AttackInput) {
//                 animator->CurrentFrame = 0;
//             }

//             if     (JumpingInput) Result = GetCharacterAction(manager, Character_Action_Jump_ID);
//             else if (MovingInput) Result = GetCharacterAction(manager, Character_Action_Walk_ID);
//             else if (AttackInput) Result = GetCharacterAction(manager, Character_Action_Attack_ID);
//         } break;
//         case Character_Action_Walk_ID: {
//             if (JumpingInput) {
//                 Result = GetCharacterAction(manager, Character_Action_Jump_ID);
//                 animator->CurrentFrame = 0;
//             }
//             else if (AttackInput) {
//                 Result = GetCharacterAction(manager, Character_Action_Attack_ID);
//                 animator->CurrentFrame = 0;
//             }
//             else if (!MovingInput) {
//                 animator->Active = false;
//                 Result = GetCharacterAction(manager, Character_Action_Idle_ID);
//             }
//         } break;
//         case Character_Action_Jump_ID: {
//             if (!animator->Active) {
//                 Result = GetCharacterAction(manager, Character_Action_Idle_ID);
//                 animator->CurrentFrame = 0;
//             }
//         } break;
//         case Character_Action_Attack_ID: {
//             if (!animator->Active) {
//                 Result = GetCharacterAction(manager, Character_Action_Idle_ID);
//                 animator->CurrentFrame = 0;
//             }
//         } break;
//         default: Raise("Invalid character action");
//     }

//     animator->Loop = Result.Loop;
//     return Result;
// }
/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Game state                                                                                                                                   |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/
const int MAX_ENTITIES = 128;

struct State {
    Free_List<Entity> entities;
    particle_emitter* emitter;
    Entity* active_camera;
    double dt;
    float time;
    bool exit;
};

Entity CreateEntity(
    const char* name, 
    Entity_type type, 
    Mesh* mesh = nullptr,
    bool active = true
) {
    Entity entity;

    // strcpy(entity.name, name);
    entity.type = type;
    entity.active = active;
    entity.mesh = mesh;

    switch(type) {
        case Camera_Entity_Type: {
            entity.angle = 0;
            entity.pitch = 0;
            entity.distance = 10.0f;
        } break;
        case Character_Entity_Type: {
            entity.flags = Entity_Flag::Render | Entity_Flag::Movable | Entity_Flag::Animated;
        } break;
        case Prop_Entity_Type: {
            entity.flags = Entity_Flag::Render;
        } break;
        case Weapon_Entity_Type: {
            entity.flags = Entity_Flag::Render;
        } break;
        default: {
            Raise("Invalid entity type.");
        }
    }

    return entity;
}

Entity* AddEntity(Free_List<Entity>& entities, Entity entity) {
    uint32 index = entities.Insert(entity);
    Entity* result = &entities[index];
    return result;
}

#endif