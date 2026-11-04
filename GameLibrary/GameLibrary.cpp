#include "pch.h"

#define STB_IMAGE_IMPLEMENTATION

#include "GameLibrary.h"
#include "GameTest.h"

#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include "xxhash.h"

// Sound
void GameOutputSound(Asset_Manager* asset_manager, game_sound_buffer* sound_buffer, State* state, Input* input) {
    
    Silence(sound_buffer);

    // DebugPlotSoundBuffer(ScreenBuffer, PreviousSoundBuffer, PreviousOrigin);
    //WriteSineWave(pSoundBuffer, 480, 0);
}

// Debug
void LogGameDebugRecords(Render_Group* group);

void TestPerformance() {
    //TIMED_BLOCK;
    
}

void UpdateGameState(
    Debug_Info* debug_info,
    Render_Group* group,
    State* state, 
    Input* input
) {
    uint32 updated_entities = 0;
    uint32 index = 0;
    while (updated_entities < state->entities.count) {
        Entity* entity = nullptr;
        if (state->entities.occupied[index]) {
            entity = &state->entities[index++];
            updated_entities++;
        }
        else {
            index++;
            continue;
        }

        if (!entity->active) continue;

        switch(entity->type) {
            case Camera_Entity_Type: {
                if (entity->active) {
                    state->active_camera = entity;
                    if (input->mode == Keyboard) {
                        // Zoom
                        if (input->mouse.wheel > 0)      entity->distance /= 1.2f;
                        else if (input->mouse.wheel < 0) entity->distance *= 1.2f;

                        // Orbit around position
                        if (
                            input->mouse.middle_click.is_down && 
                            input->mouse.middle_click.was_down &&
                            input->mouse.cursor.x >= 0 && input->mouse.cursor.x <= group->width &&
                            input->mouse.cursor.y >= 0 && input->mouse.cursor.y <= group->height
                        ) {
                            V2 offset = input->mouse.cursor - input->mouse.last_cursor;
                            double angular_velocity = 0.5;
                
                            entity->angle -= angular_velocity * offset.x;
                            entity->pitch += angular_velocity * offset.y;
                        }
                    }

                    if (input->mode == Controller) {
                        V2 joystic = input->controller.right_joystick;

                        if (modulus(joystic) > 0.1) {
                            entity->angle -= 3.0 * joystic.x;
                            entity->pitch -= 3.0 * joystic.y;
                        }
                    }

                    entity->angle = NormalizeAngle(entity->angle);
                    entity->pitch = NormalizeAngle(entity->pitch);
                    entity->transform.translation = entity->anchor + entity->distance * Radial(entity->angle, entity->pitch);
                }
            } break;

            case Character_Entity_Type: {
                // Collision
                entity->collided = false;
                uint32 tested_entities = 0;
                uint32 test_index = 0;
                while (tested_entities < state->entities.count) {
                    Entity* test_entity = nullptr;
                    if (state->entities.occupied[test_index]) {
                        test_entity = &state->entities[test_index++];
                        tested_entities++;
                    }
                    else {
                        test_index++;
                        continue;
                    }

                    if (test_entity == entity) {
                        continue;
                    }

                    if (Collide(entity, test_entity)) {
                        entity->collided = true;
                        test_entity->collided = true;
                    }
                    else {
                        test_entity->collided = false;
                    }
                }
            } break;

            case Prop_Entity_Type: {

            } break;

            case Weapon_Entity_Type: {
                if (entity->parent == nullptr) {
                    entity->angular_velocity = V3(0,1,0);
                }
            } break;

            default: {
                Raise("Invalid entity type.");
            }
        }

        // Movement by input
        if (entity->flags & Entity_Flag::Movable) {
            V3 direction = V3(0,0,0);
            float speed = 20.0f;
            if (input->mode == Keyboard) {
                bool left = input->keyboard.A.is_down;
                bool right = input->keyboard.D.is_down;
                bool up = input->keyboard.W.is_down;
                bool down = input->keyboard.S.is_down;
                if (right) { direction.x += 1.0f; }
                if (left)  { direction.x -= 1.0f; }
                if (up)    { direction.z += 1.0f; }
                if (down)  { direction.z -= 1.0f; }
                direction = normalize(direction);
                if (input->keyboard.space.is_down) direction.y += 1.0f;
                if (input->keyboard.shift.is_down) direction.y -= 1.0f;
            }
            else if (input->mode == Controller) {
                V2 normalized = normalize(input->controller.left_joystick);
                direction.x = normalized.x;
                direction.z = normalized.y;
                speed = 20.0f * modulus(input->controller.left_joystick);
            }

            if (modulus(direction) > EPSILON) {
                Basis horizontal_basis = GetCameraBasis(state->active_camera->angle, 0);
                // Direction is in coordinates relative to camera
                float angle = atan2f(-direction.x, -direction.z);
                direction = direction.y * V3(0.0, 1.0, 0.0) + direction.x * horizontal_basis.x + direction.z * horizontal_basis.z;
                entity->velocity = speed * direction;
                entity->transform.rotation = BuildQuaternion(angle - state->active_camera->angle * DEGREES, V3(0,1,0));
            }
            else {
                entity->velocity = V3(0,0,0);
            }
        }
        // Movable flag overrides follow mechanic
        else if (entity->follow) {
            V3 displacement = entity->follow->transform.translation - entity->anchor;
            displacement.y = 0;
            float distance = modulus(displacement);
            float min_distance = .01f;
            if (distance >= min_distance) entity->velocity = 20.0f * (distance - min_distance) * normalize(displacement);
            else                          entity->velocity = V3(0,0,0);
        }

        // Kinematics
        if (entity->velocity != V3(0,0,0)) {
            entity->transform.translation += state->dt * entity->velocity;
            entity->anchor += state->dt * entity->velocity;
        }
        if (entity->angular_velocity != V3(0,0,0)) {
            entity->transform.rotation = entity->transform.rotation * BuildQuaternion(state->dt, entity->angular_velocity);
        }
        
        // Animation
        if (entity->flags & Entity_Flag::Animated) {
            // TODO
        }

        // Rendering
        if (entity->flags & Entity_Flag::Render) {
            Entity* camera = state->active_camera;
            Basis basis = GetCameraBasis(camera->angle, camera->pitch);
            Ray ray = MouseRay(group->width, group->height, camera->anchor - camera->distance * basis.z, basis, input->mouse.cursor);
            Collider collider = entity->transform * entity->collider;
            entity->hovered = Raycast(ray, collider);
            bool outline = entity->hovered;
            // PushMesh(
            //     Group, 
            //     Entity->Mesh, 
            //     .Color = Entity->Color,
            //     .Texture = Entity->Texture, 
            //     .Transform = Entity->Transform, 
            //     .Outline = Outline
            // );

            if (group->debug && group->debug_colliders) {
                // PushCollider(group, entity->collider, entity->transform, entity->collided ? Color::RED : Color::YELLOW);
            }
        }
    }
}

// Main
extern "C" GAME_UPDATE(GameUpdate)
{
    if (memory->hot_reload) {
        rng.seed = SeedRNG(memory->transient);
        rng.state = rng.seed;

        time_records = (Time_Record*)&memory->time_records_library;

        if (memory->test) {
            // RunReloadTests(memory);
        }

#ifdef _WIN32
        LARGE_INTEGER perf_count_frequency_result;
        QueryPerformanceFrequency(&perf_count_frequency_result);
        Platform.performance_counter_frequency = perf_count_frequency_result.QuadPart;
#endif
        
        memory->hot_reload = false;
    }

    Render_Group* group = &memory->render_group;
    Input* input = &memory->input;
    State* state = memory->state;
    Asset_Manager* asset_manager = &memory->asset_manager;
    Debug_Info* debug_info = &memory->debug_info;
{
    TIMED_BLOCK;

    float time = state->time;
    Entity* active_camera = state->active_camera;

    bool first_frame = false;
    if (!memory->initialized) {
        first_frame = true;

        // Initialize entities
        state->entities = Free_List<Entity>(memory->permanent, MAX_ENTITIES);

        Entity* camera = AddEntity(state->entities, CreateEntity("Camera", Camera_Entity_Type));
        state->active_camera = camera;
        camera->angle = -45.0f;
        camera->pitch = 22.5f;
        camera->anchor = V3(0, 3.2f, 0);

        if (memory->test) {
            // RunOnceTests(memory);
        }

        state->emitter = AllocateParticleEmitter(memory->permanent, 200);
        SetParticleEmitterCircle(state->emitter, V3(0,0,0), 1.0f, V3(0,1,0));
        state->emitter->ParticleLifetime = 2.0f;

        memory->initialized = true;
    }

    // PushClear(group, Color::BLACK, Target_None);
    // PushClear(group, Color::EMPTY, Target_World);
    // PushClear(group, Color::EMPTY, Target_Outline);
    // PushClear(group, Color::EMPTY, Target_Postprocessing_Outline);
    // PushClear(group, Color::MAGENTA, Target_PingPong);
    // PushClear(group, Color::BLACK, Target_Output);

    if (memory->test) {
        // RunEveryFrameTests(Memory);
    }

    UpdateGameState(debug_info, group, state, input);

    // PushSky(Group);
    
    // GameOutputSound(Assets, SoundBuffer, State, Input);
    
    // UpdateUI(Memory, Input);

    // PushRenderTarget(Group, Target_World, Target_Output);

    group->light.camera_position = state->active_camera->transform.translation;

    static bool screenshot = false;
    if (input->keyboard.F10.was_down && !input->keyboard.F10.is_down) {
        screenshot = true;
    }

    static double screen_rect_alpha = 1.0;
    if (screenshot) {
        screen_rect_alpha -= 0.05;
        if (screen_rect_alpha < 0.0) {
            screenshot = false;
            screen_rect_alpha = 1.0;
        }
        else {
            Rect screen_rect = { 0, 0, (float)group->width, (float)group->height };
            // PushRect(group, screen_rect, ChangeAlpha(Color::WHITE, ScreenRectAlpha), SORT_ORDER_PUSH_RENDER_TARGETS - 5.0);
        }
    }
    // PushRenderTarget(Group, Target_Output, Target_None, SORT_ORDER_PUSH_RENDER_TARGETS + 100.0);
}
    memory->time_records_library_count = __COUNTER__;
    if (group->debug) {
        // PushTimeRecords(
        //     Group, 
        //     Memory->nTimeRecordsLibrary, Memory->TimeRecordsLibrary, 
        //     Memory->nTimeRecordsPlatform, Memory->TimeRecordsPlatform
        // );
    }
}
