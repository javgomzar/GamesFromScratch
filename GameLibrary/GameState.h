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
    entity_flag Flags;
    game_entity_type Type;
    game_entity* Parent = nullptr;
    game_entity* Follow = nullptr;
    collider Collider;
    color Color = White;

    transform Transform = IdentityTransform;
    v3 Velocity = V3(0,0,0);
    v3 AngularVelocity = V3(0,0,0);

    // Rendering
    game_mesh_id MeshID;
    game_bitmap_id TextureID = Bitmap_Empty_ID;

    // View
    v3 Anchor = V3(0,0,0);
    float Distance = 0.0f;
    float Pitch = 0.0f;
    float Angle = 0.0f;

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

basis GetBasis(float Angle, float Pitch) {
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

matrix4 GetViewMatrix(matrix3 Basis, float Distance, v3 Position) {
	Basis.Z = -Basis.Z;
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

enum character_class {
    Knight_Class,
    Rogue_Class,
    Hunter_Class,
    Wizard_Class,
    Bard_Class,
    Priest_Class
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
    game_entity Entity = {};

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

void UpdateGameState(render_group* Group, game_state* State, game_input* Input, float Width, float Height) {
    uint32 UpdatedEntities = 0;
    uint32 Index = 0;
    while (UpdatedEntities < State->Entities.Count) {
        game_entity* Entity = nullptr;
        if (State->Entities.IsOccupied[Index]) {
            Entity = &State->Entities[Index++];
            UpdatedEntities++;
        }
        else {
            Index++;
            continue;
        }

        if (!Entity->Active) continue;

        switch(Entity->Type) {
            case Camera_Entity_Type: {
                if (Entity->Active) {
                    State->ActiveCamera = Entity;
                    if (Input->Mode == Keyboard) {
                        // Zoom
                        if (Input->Mouse.Wheel > 0)      Entity->Distance /= 1.2f;
                        else if (Input->Mouse.Wheel < 0) Entity->Distance *= 1.2f;

                        // Orbit around position
                        if (
                            Input->Mouse.MiddleClick.IsDown && 
                            Input->Mouse.MiddleClick.WasDown &&
                            Input->Mouse.Cursor.X >= 0 && Input->Mouse.Cursor.X <= Width &&
                            Input->Mouse.Cursor.Y >= 0 && Input->Mouse.Cursor.Y <= Height
                        ) {
                            v2 Offset = Input->Mouse.Cursor - Input->Mouse.LastCursor;
                            double AngularVelocity = 0.5;
                
                            Entity->Angle -= AngularVelocity * Offset.X;
                            Entity->Pitch += AngularVelocity * Offset.Y;
                        }
                    }

                    if (Input->Mode == Controller) {
                        v2 Joystic = V2(Input->Controller.RightJoystick.X, Input->Controller.RightJoystick.Y);

                        if (modulus(Joystic) > 0.1) {
                            Entity->Angle -= 3.0 * Joystic.X;
                            Entity->Pitch -= 3.0 * Joystic.Y;
                        }
                    }

                    // Camera basis
                    // Entity->Basis = GetCameraBasis(Entity->Angle, Entity->Pitch);
                    // Entity->View = GetViewMatrix(Entity->Basis, Entity->Distance, Entity->Position);
                }
            } break;

            case Character_Entity_Type: {

            } break;

            case Prop_Entity_Type: {

            } break;

            case Weapon_Entity_Type: {

            } break;

            default: {
                Raise("Invalid entity type.");
            }
        }

        // Movement
        if (Entity->Flags & entity_flag::movable) {
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

            if (modulus(Direction) > Epsilon) {
                basis HorizontalBasis = GetBasis(State->ActiveCamera->Angle, 0);
                // Direction is in coordinates relative to camera
                float Angle = atan2f(-Direction.X, Direction.Z);
                Direction = Direction.Y * V3(0.0, 1.0, 0.0) + Direction.X * HorizontalBasis.X - Direction.Z * HorizontalBasis.Z;
                Entity->Velocity = Speed * Direction;
                Entity->Transform.Rotation = Quaternion(State->ActiveCamera->Angle * Degrees + Angle, V3(0,1,0));
                Entity->Transform.Translation += State->dt * Entity->Velocity;
                Entity->Anchor += State->dt * Entity->Velocity;
            }
        }
        // Movable flag overrides follow mechanic
        else if (Entity->Follow) {
            v3 Displacement = Entity->Follow->Transform.Translation - Entity->Transform.Translation;
            Displacement.Y = 0;
            float Distance = modulus(Displacement);
            v3 Velocity = V3(0,0,0);
            float MinDistance = .01f;
            if (Distance >= MinDistance) Velocity = 20.0f * (Distance - MinDistance) * normalize(Displacement);
            Entity->Anchor += State->dt * Velocity;
            Entity->Transform.Translation += State->dt * Velocity;
        }

        // Animation
        if (Entity->Flags & entity_flag::animated) {
            // TODO
        }

        if (Entity->Flags & entity_flag::render) {
            game_entity* Camera = State->ActiveCamera;
            basis Basis = GetBasis(Camera->Angle, Camera->Pitch);
            ray Ray = MouseRay(Group->Width, Group->Height, Camera->Anchor + Camera->Distance * Basis.Z, Basis, Input->Mouse.Cursor);
            collider Collider = Entity->Transform * Entity->Collider;
            Entity->Hovered = Raycast(Ray, Collider);
            bool Outline = Entity->Hovered;
            PushMesh(
                Group, 
                Entity->MeshID, 
                .Color = Entity->Color,
                .TextureID = Entity->TextureID, 
                .Transform = Entity->Transform, 
                .Outline = Outline
            );

            if (Group->Debug && Group->DebugColliders) {
                PushCollider(Group, Entity->Collider, Entity->Transform, Entity->Collided ? Red : Yellow);
            }
        }
    }
}

#endif