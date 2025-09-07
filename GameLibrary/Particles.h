#include "GameMath.h"
#include "GameAssets.h"
#include "GameRender.h"

#ifndef PARTICLES
#define PARTICLES

struct particle {
    color Color;
    v3 Position;
    v3 Velocity;
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
        v3 Point;
        segment3 Segment;
        struct {
            v3 Center;
            float Radius;
            v3 Normal;
        } Circle;
        struct {
            v3 Center;
            float Radius;
        } Sphere;
    };
    uint32 Size;
    uint32 Count;
    float ParticleLifetime;
    particle Particles[];
};

particle_emitter* AllocateParticleEmitter(memory_arena* Arena, uint32 N) {
    particle_emitter* Result = PushStruct(Arena, particle_emitter);
    Result->Size = N;
    Result->Count = 0;
    PushArray(Arena, N, particle);
    return Result;
}

void SetParticleEmitterPoint(particle_emitter* Emitter, v3 Position) {
    Emitter->Type = particle_emitter_point;
    Emitter->Point = Position;
}

void SetParticleEmitterSegment(particle_emitter* Emitter, v3 Head, v3 Tail) {
    Emitter->Type = particle_emitter_segment;
    Emitter->Segment = {Head, Tail};
}

void SetParticleEmitterCircunference(particle_emitter* Emitter, v3 Center, float Radius, v3 Normal) {
    Emitter->Type = particle_emitter_circunference;
    Emitter->Circle = { Center, Radius, Normal };
}

void SetParticleEmitterCircle(particle_emitter* Emitter, v3 Center, float Radius, v3 Normal) {
    Emitter->Type = particle_emitter_circle;
    Emitter->Circle = { Center, Radius, Normal };
}

void SetParticleEmitterSphere(particle_emitter* Emitter, v3 Center, float Radius) {
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
            Particle->Position.X = Emitter->Point.X;
            Particle->Position.Z = Emitter->Point.Z;
        } break;
        case particle_emitter_segment: {
            float t = RandFloat();
            Particle->Position = t * Emitter->Segment.Head + (1 - t) * Emitter->Segment.Tail;
        } break;
        case particle_emitter_circunference: {
            basis Basis = Complete(Emitter->Circle.Normal);
            float Angle = RandFloat(0, Tau);
            Particle->Position = Emitter->Circle.Radius * (cosf(Angle) * Basis.Y + sinf(Angle) * Basis.Z);
            Particle->Velocity = V3(0, RandFloat(0.01, 0.03), 0);
        } break;
        case particle_emitter_circle: {
            basis Basis = Complete(Emitter->Circle.Normal);
            float R = sqrt(RandFloat(0, Emitter->Circle.Radius));
            float Angle = RandFloat(0, Tau);
            Particle->Position = R * (cosf(Angle) * Basis.Y + sinf(Angle) * Basis.Z);
            Particle->Velocity = V3(0, RandFloat(0.01, 0.03), 0);
        } break;
    }
}

void Update(render_group* Group, particle_emitter* Emitter, float dt) {
    TIMED_BLOCK;
    
    AddParticle(Emitter);

    for (int i = 0; i < Emitter->Size; i++) {
        particle* Particle = &Emitter->Particles[i];
        if (Particle->Time > 0) {
            v3 AngularVelocity = 0.01f * cross(Emitter->Circle.Normal, Particle->Position);
            Particle->Velocity.X = AngularVelocity.X;
            Particle->Velocity.Z = AngularVelocity.Z;

            Particle->Position += Particle->Velocity;
            Particle->Color = HSV2RGB(1.0f + 0.2f * (Particle->Time / Emitter->ParticleLifetime), 1.0f, 1.0f, 1.0f);
            PushPoint(Group, Particle->Position, Particle->Color, SORT_ORDER_DEBUG_OVERLAY - distance(Particle->Position, Group->Camera->Position));

            Particle->Time -= dt;
            if (Particle->Time < 0) Particle->Time = 0;
        }
    }
}

#endif