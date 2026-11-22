#pragma once

#include "input.h"
#include <render.h>
// #include "particles.h"
#include <geometry.h>
#include <animation.h>

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
    Entity_Flag flags = Entity_Flag::None;
    Entity_type type;
    Entity* parent = nullptr;
    Entity* follow = nullptr;
    Collider collider = SphereCollider(V3(0,0,0), 1.0f);
    Color color = Color::WHITE;

    Transform transform = Transform::ID;
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

bool Collide(Entity* entity1, Entity* entity2);

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Camera                                                                                                                                       |
+----------------------------------------------------------------------------------------------------------------------------------------------+s
*/

Basis GetCameraBasis(float angle, float pitch);
Matrix4 GetViewMatrix(Matrix3 basis, float distance, V3 position);
V2 GetScreenPosition(float screen_width, float screen_height, Matrix4 view, V3 world_position);
Ray MouseRay(float width, float height, V3 camera_position, Basis camera_basis, V2 mouse);

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Weapons                                                                                                                                      |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/

// Transform WeaponTransforms[] = {
//     GetTransform(
//         BuildV3(0.5f,2.0f,0),
//         BuildQuaternion(-0.25f * TAU, BuildV3(0,1,0)) * BuildQuaternion(-0.25f * TAU, V3(1,0,0))
//     ),
//     GetTransform(
//         BuildV3(-0.7f,2.2f,0),
//         BuildQuaternion(0.5f * TAU, BuildV3(0,0,1)) * BuildQuaternion(0.25f * TAU, V3(1,0,0))
//     ),
// };

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
inline const int MAX_ENTITIES = 128;

struct State {
    Free_List<Entity> entities;
    // particle_emitter* emitter;
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
);

Entity* AddEntity(Free_List<Entity>& entities, Entity entity);
