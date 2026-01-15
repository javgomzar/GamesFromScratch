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
    Entity_Type_Camera
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
    transform Transform = GetTransform(V3(0,0,0)),
    v3 Velocity = V3(0,0,0),
    collider Collider = SphereCollider(V3(0.0f, 0.0f, 0.0f), 1.0f),
    bool Active = true
) {
    game_entity Result = {0};
    Result.ID = -1;
    strcpy_s(Result.Name, Name);
    Result.Transform = Transform;
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
// | Entity List                                                                                                                                  |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

const int MAX_ENTITIES = 16;
DefineFreeList(MAX_ENTITIES, game_entity);

struct game_entity_state {
    game_entity_list Entities;
    camera_list Cameras;
};

game_entity* AddEntity(
    game_entity_state* State,
    const char* Name,
    game_entity_type Type,
    collider Collider,
    v3 Position = V3(0,0,0),
    quaternion Rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f),
    scale Scale = GetScale(),
    bool Active = true
) {
    Assert(State->Entities.Count < MAX_ENTITIES);

    // If any ID is free, use it
    game_entity* Entity = Insert(&State->Entities);
    Entity->Type = Type;
    Entity->Transform = GetTransform(Position, Rotation, Scale);
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
    game_entity* Entity = AddEntity(
        State, 
        NameBuffer, 
        Entity_Type_Camera, 
        SphereCollider(Position, 1.0f), 
        Position, 
        Rotation, 
        GetScale(), 
        Cam->ID == 0
    );
    Entity->Index = Cam->ID;
    Cam->Entity = Entity;

    return Cam;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Stars                                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct star {
    char Name[64];
    int Index;
    float Hue;
    float Intensity;
    float RightAscension;
    float Declination;
};

const int MAX_STARS = 1024;

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Game state                                                                                                                                   |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct game_state {
    game_entity_state Entities;
    particle_emitter* Emitter;
    camera* ActiveCamera;
    int nStars;
    star Stars[MAX_STARS];
    float* StarsBuffer;
    float Latitude;
    float Longitude;
    double dt;
    float Time;
    bool Exit;
    bool Debug;
};

void Initialize(game_state* State, memory_arena* Arena) {
    State->Latitude = 45;
    State->Longitude = 0;

    // Initialize stars
    // State->nStars = 1;
    // star* Star = State->Stars;
    // Star->Index = 0;
    // sprintf_s(Star->Name, "Star %d", 0);
    // Star->RightAscension = 0;
    // Star->Declination = 45;

    State->nStars = 700;
    for (int i = 0; i < State->nStars; i++) {
        star* Star = &State->Stars[i];
        Star->Index = i;
        sprintf_s(Star->Name, "Star %d", i);
        Star->RightAscension = RandFloat(0.0f, 24.0f);
        Star->Declination = RandFloat(0.0f, 90.0f);
    }

    State->StarsBuffer = PushArray(Arena, MAX_STARS * 7, float);
}

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

            if (!State->Debug) {
                if (Cam->Pitch > 0.0f) {
                    Cam->Pitch = 0.0f;
                }
    
                if (Cam->Pitch < -90.0f) {
                    Cam->Pitch = -90.0f;
                }
            }
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
}

void PushStars(render_group* Group, game_state* State) {
    float* Vertices = (float*)PushPrimitiveCommand(
        Group, 
        render_primitive_triangle, 
        White, 
        vertex_layout_v2_v2_id,
        6,
        0,
        SORT_ORDER_DEBUG_OVERLAY,
        {
            .Flags = STAR_FLAG
        }
    )->Vertices;

    *Vertices++ = -1.0f; *Vertices++ = -1.0f; *Vertices++ = -1.0f; *Vertices++ = -1.0f;
    *Vertices++ =  1.0f; *Vertices++ = -1.0f; *Vertices++ =  1.0f; *Vertices++ = -1.0f;
    *Vertices++ = -1.0f; *Vertices++ =  1.0f; *Vertices++ = -1.0f; *Vertices++ =  1.0f;
    *Vertices++ =  1.0f; *Vertices++ = -1.0f; *Vertices++ =  1.0f; *Vertices++ = -1.0f;
    *Vertices++ = -1.0f; *Vertices++ =  1.0f; *Vertices++ = -1.0f; *Vertices++ =  1.0f;
    *Vertices++ =  1.0f; *Vertices++ =  1.0f; *Vertices++ =  1.0f; *Vertices++ =  1.0f;

    for (int i = 0; i < State->nStars; i++) {
        State->StarsBuffer[7*i]   = State->Stars[i].RightAscension;
        State->StarsBuffer[7*i+1] = State->Stars[i].Declination;
        State->StarsBuffer[7*i+2] = 100.0f; // Size
        State->StarsBuffer[7*i+3] = 1.0f; // Color R
        State->StarsBuffer[7*i+4] = 1.0f; // Color G
        State->StarsBuffer[7*i+5] = 1.0f; // Color B
        State->StarsBuffer[7*i+6] = 1.0f; // Color A
    }
}

#endif