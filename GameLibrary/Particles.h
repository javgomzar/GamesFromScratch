#include "GameMath.h"
#include "GameAsset.h"
#include "GameRender.h"

#ifndef PARTICLES
#define PARTICLES

struct particle {
    Color color;
    V3 Position;
    V3 Velocity;
    float Time;
};

enum particle_emitter_type {
    particle_emitter_point,
    particle_emitter_segment,
    particle_emitter_circunference,
    particle_emitter_circle,
    particle_emitter_sphere,
};

struct particle_emitter {
    particle_emitter_type Type;
    union {
        V3 Point;
        Segment3 Segment;
        struct {
            V3 Center;
            float Radius;
            V3 Normal;
        } Circle;
        struct {
            V3 Center;
            float Radius;
        } Sphere;
    };
    uint32 Size;
    uint32 Count;
    float ParticleLifetime;
    particle Particles[];
};

particle_emitter* AllocateParticleEmitter(Arena& Arena, uint32 N) {
    particle_emitter* Result = PushStruct(Arena, particle_emitter);
    Result->Size = N;
    Result->Count = 0;
    PushArray(Arena, N, particle);
    return Result;
}

void SetParticleEmitterPoint(particle_emitter* Emitter, V3 Position) {
    Emitter->Type = particle_emitter_point;
    Emitter->Point = Position;
}

void SetParticleEmitterSegment(particle_emitter* Emitter, V3 Head, V3 Tail) {
    Emitter->Type = particle_emitter_segment;
    Emitter->Segment = {Head, Tail};
}

void SetParticleEmitterCircunference(particle_emitter* Emitter, V3 Center, float Radius, V3 Normal) {
    Emitter->Type = particle_emitter_circunference;
    Emitter->Circle = { Center, Radius, Normal };
}

void SetParticleEmitterCircle(particle_emitter* Emitter, V3 Center, float Radius, V3 Normal) {
    Emitter->Type = particle_emitter_circle;
    Emitter->Circle = { Center, Radius, Normal };
}

void SetParticleEmitterSphere(particle_emitter* Emitter, V3 Center, float Radius) {
    Emitter->Type = particle_emitter_sphere;
    Emitter->Sphere = { Center, Radius };
}

void AddParticle(particle_emitter* Emitter) {
    if (Emitter->Count >= Emitter->Size) {
        Emitter->Count = 0;
    }
    particle* Particle = &Emitter->Particles[Emitter->Count++];
    Particle->Time = Emitter->ParticleLifetime;
    switch(Emitter->Type) {
        case particle_emitter_point: {
            Particle->Position.x = Emitter->Point.x;
            Particle->Position.z = Emitter->Point.z;
        } break;
        case particle_emitter_segment: {
            float t = RandFloat();
            Particle->Position = t * Emitter->Segment.head + (1 - t) * Emitter->Segment.tail;
        } break;
        case particle_emitter_circunference: {
            Basis basis = Complete(Emitter->Circle.Normal);
            float Angle = RandFloat(0, TAU);
            Particle->Position = Emitter->Circle.Radius * (cosf(Angle) * basis.y + sinf(Angle) * basis.z);
            Particle->Velocity = V3(0, RandFloat(0.01, 0.03), 0);
        } break;
        case particle_emitter_circle: {
            Basis basis = Complete(Emitter->Circle.Normal);
            float R = sqrt(RandFloat(0, Emitter->Circle.Radius));
            float Angle = RandFloat(0, TAU);
            Particle->Position = R * (cosf(Angle) * basis.y + sinf(Angle) * basis.z);
            Particle->Velocity = V3(0, RandFloat(0.01, 0.03), 0);
        } break;
        default: {
            Raise("Invalid emitter type.");
        }
    }
}

// void Update(render_group* Group, V3 CameraPosition, particle_emitter* Emitter, float dt) {
//     V3* Vertices = (V3*)PushPrimitiveCommand(Group, render_primitive_point, vertex_layout_V3_id, Emitter->Count)->Vertices;

//     for (int i = 0; i < Emitter->Size; i++) {
//         particle* Particle = &Emitter->Particles[i];
//         if (Particle->Time > 0) {
//             V3 AngularVelocity = 0.01f * cross(Emitter->Circle.Normal, Particle->Position);
//             Particle->Velocity.x = AngularVelocity.x;
//             Particle->Velocity.z = AngularVelocity.z;

//             Particle->Position += Particle->Velocity;
//             Particle->Color = HSV2RGB(1.0f + 0.2f * (Particle->Time / Emitter->ParticleLifetime), 1.0f, 1.0f, 1.0f);
//             PushPoint(Group, Particle->Position, Particle->Color, SORT_ORDER_DEBUG_OVERLAY - distance(Particle->Position, CameraPosition));

//             Particle->Time -= dt;
//             if (Particle->Time < 0) Particle->Time = 0;
//         }
//     }
// }

#endif