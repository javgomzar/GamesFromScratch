#include "pch.h"
#include "GameLibrary.h"

#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include "xxhash.h"

// Sound
void GameOutputSound(game_assets* Assets, game_sound_buffer* pSoundBuffer, game_state* pGameState, game_input* Input) {
    
    Silence(pSoundBuffer);

    // DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
    //WriteSineWave(pSoundBuffer, 480, 0);
}

// Debug
void LogGameDebugRecords(render_group* Group);

void TestPerformance() {
    //TIMED_BLOCK;
    
}

void UpdateGameState(
    debug_info* DebugInfo, 
    render_group* Group, 
    game_state* State, 
    game_input* Input
) {
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
                            Input->Mouse.Cursor.X >= 0 && Input->Mouse.Cursor.X <= Group->Width &&
                            Input->Mouse.Cursor.Y >= 0 && Input->Mouse.Cursor.Y <= Group->Height
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

                    Entity->Angle = NormalizeAngle(Entity->Angle);
                    Entity->Pitch = NormalizeAngle(Entity->Pitch);
                    Entity->Transform.Translation = Entity->Anchor + Entity->Distance * Radial(Entity->Angle, Entity->Pitch);
                }
            } break;

            case Character_Entity_Type: {
                // Collision
                Entity->Collided = false;
                uint32 TestedEntities = 0;
                uint32 TestIndex = 0;
                while (TestedEntities < State->Entities.Count) {
                    game_entity* TestEntity = nullptr;
                    if (State->Entities.IsOccupied[TestIndex]) {
                        TestEntity = &State->Entities[TestIndex++];
                        TestedEntities++;
                    }
                    else {
                        TestIndex++;
                        continue;
                    }

                    if (TestEntity == Entity) {
                        continue;
                    }

                    if (Collide(Entity, TestEntity)) {
                        Entity->Collided = true;
                        TestEntity->Collided = true;
                    }
                    else {
                        TestEntity->Collided = false;
                    }
                }
            } break;

            case Prop_Entity_Type: {

            } break;

            case Weapon_Entity_Type: {
                if (Entity->Parent == nullptr) {
                    Entity->AngularVelocity = V3(0,1,0);
                }
            } break;

            default: {
                Raise("Invalid entity type.");
            }
        }

        // Movement by input
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
                basis HorizontalBasis = GetCameraBasis(State->ActiveCamera->Angle, 0);
                // Direction is in coordinates relative to camera
                float Angle = atan2f(-Direction.X, -Direction.Z);
                Direction = Direction.Y * V3(0.0, 1.0, 0.0) + Direction.X * HorizontalBasis.X + Direction.Z * HorizontalBasis.Z;
                Entity->Velocity = Speed * Direction;
                Entity->Transform.Rotation = Quaternion(Angle - State->ActiveCamera->Angle * Degrees, V3(0,1,0));
            }
            else {
                Entity->Velocity = V3(0,0,0);
            }
        }
        // Movable flag overrides follow mechanic
        else if (Entity->Follow) {
            v3 Displacement = Entity->Follow->Transform.Translation - Entity->Anchor;
            Displacement.Y = 0;
            float Distance = modulus(Displacement);
            float MinDistance = .01f;
            if (Distance >= MinDistance) Entity->Velocity = 20.0f * (Distance - MinDistance) * normalize(Displacement);
            else                         Entity->Velocity = V3(0,0,0);
        }

        // Kinematics
        if (Entity->Velocity != V3(0,0,0)) {
            Entity->Transform.Translation += State->dt * Entity->Velocity;
            Entity->Anchor += State->dt * Entity->Velocity;
        }
        if (Entity->AngularVelocity != V3(0,0,0)) {
            Entity->Transform.Rotation = Entity->Transform.Rotation * Quaternion(State->dt, Entity->AngularVelocity);
        }
        
        // Animation
        if (Entity->Flags & entity_flag::animated) {
            // TODO
        }

        // Rendering
        if (Entity->Flags & entity_flag::render) {
            game_entity* Camera = State->ActiveCamera;
            basis Basis = GetCameraBasis(Camera->Angle, Camera->Pitch);
            ray Ray = MouseRay(Group->Width, Group->Height, Camera->Anchor - Camera->Distance * Basis.Z, Basis, Input->Mouse.Cursor);
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

// Main
extern "C" GAME_UPDATE(GameUpdate)
{
    if (Memory->HotReload) {
        RNG.Seed = SeedRNG();
        RNG.State = RNG.Seed;

        TimeRecords = (time_record*)&Memory->TimeRecordsLibrary;

#ifdef _WIN32
        LARGE_INTEGER PerfCountFrequencyResult;
        QueryPerformanceFrequency(&PerfCountFrequencyResult);
        Platform.PerformanceCounterFrequency = PerfCountFrequencyResult.QuadPart;
#endif
        
        Memory->HotReload = false;
    }

    render_group* Group = &Memory->RenderGroup;
    game_input* Input = &Memory->Input;
    game_state* State = Memory->GameState;
    game_assets* Assets = &Memory->Assets;
    debug_info* DebugInfo = &Memory->DebugInfo;
{
    TIMED_BLOCK;

    float Time = State->Time;
    game_entity* ActiveCamera = State->ActiveCamera;

    bool FirstFrame = false;
    if (!Memory->IsInitialized) {
        FirstFrame = true;

        // TestPerformance();

        TestData();
        TestDataFileManager();

        // Initialize entities
        State->Entities = free_list<game_entity>(&Memory->Permanent, MAX_ENTITIES);

        TestEntities(State);

        State->Emitter = AllocateParticleEmitter(&Memory->Permanent, 200);
        SetParticleEmitterCircle(State->Emitter, V3(0,0,0), 1.0f, V3(0,1,0));
        State->Emitter->ParticleLifetime = 2.0f;

        Memory->IsInitialized = true;
    }

    PushClear(Group, Black, Target_None);
    PushClear(Group, { 0 }, Target_World);
    PushClear(Group, { 0 }, Target_Outline);
    PushClear(Group, { 0 }, Target_Postprocessing_Outline);
    PushClear(Group, Magenta, Target_PingPong);
    PushClear(Group, Black, Target_Output);

    UpdateGameState(DebugInfo, Group, State, Input);

    PushSky(Group);
    
    // GameOutputSound(Assets, SoundBuffer, State, Input);

    // PushEntities(Group, ActiveCamera, State, Input, Time);

    // TestRendering(Group, Input, Time);

    // TestInstancedRendering(Group);

    // TestFluid(Group, Input, FirstFrame);

    // TestSky(State->Time, &Group->Light);

    // TestFFT(Group, &Memory->Permanent, Time);

    // Update(Group, ActiveCamera->Position, State->Emitter, State->dt);
    
    UpdateUI(Memory, Input);

    PushRenderTarget(Group, Target_World, Target_Output);

    Group->Light.CameraPosition = State->ActiveCamera->Transform.Translation;

    static bool Screenshot = false;
    if (Input->Keyboard.F10.WasDown && !Input->Keyboard.F10.IsDown) {
        Screenshot = true;
    }

    static double ScreenRectAlpha = 1.0;
    if (Screenshot) {
        ScreenRectAlpha -= 0.05;
        if (ScreenRectAlpha < 0.0) {
            Screenshot = false;
            ScreenRectAlpha = 1.0;
        }
        else {
            rectangle ScreenRect = { 0, 0, (float)Group->Width, (float)Group->Height };
            PushRect(Group, ScreenRect, ChangeAlpha(White, ScreenRectAlpha), SORT_ORDER_PUSH_RENDER_TARGETS - 5.0);
        }
    }
    PushRenderTarget(Group, Target_Output, Target_None, SORT_ORDER_PUSH_RENDER_TARGETS + 100.0);
}
    Memory->nTimeRecordsLibrary = __COUNTER__;
    if (Group->Debug) {
        PushTimeRecords(
            Group, 
            Memory->nTimeRecordsLibrary, Memory->TimeRecordsLibrary, 
            Memory->nTimeRecordsPlatform, Memory->TimeRecordsPlatform
        );
    }
}
