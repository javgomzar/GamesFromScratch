#include "pch.h"
#include "entity.h"


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
    Get vector basis for a camera given its angle and pitch. Relative to the camera:
        - The X axis is left to right.
        - The Y axis is down to up.
        - The Z axis is back to front.
    Note that this basis is left-handed.
*/
Basis GetCameraBasis(float angle, float pitch) {
    float cosA = cosf(angle * DEGREES);
    float sinA = sinf(angle * DEGREES);
    float cosP = cosf(pitch * DEGREES);
    float sinP = sinf(pitch * DEGREES);

    return Basis(
        V3(        cosA,  0.0,        -sinA),
        V3(-sinA * sinP, cosP, -cosA * sinP),
        V3(-sinA * cosP,-sinP, -cosA * cosP)
    );
}

Matrix4 GetViewMatrix(Matrix3 basis, float distance, V3 position) {
	basis = basis.Transpose();
	V3 translation = V3(0, 0, distance) - position * basis;
	return Matrix4(
        V4(basis.x, 0),
        V4(basis.y, 0),
        V4(basis.z, 0),
        V4(translation, 1)
    );
}

V2 GetScreenPosition(float screen_width, float screen_height, Matrix4 view, V3 world_position) {
    V4 point = V4(world_position, 1);
    Matrix4 projection = GetWorldProjectionMatrix(screen_width, screen_height);
    V4 view_point = point * view * projection;
    float Factor = view_point.w == 0 ? 0 : 1.0f/view_point.w;
    V2 device_point = Factor * V2(view_point.x, view_point.y);
    V2 screen_point = V2(
        0.5f * (1.0f + device_point.x) * screen_width,
        0.5f * (1.0f - device_point.y) * screen_height
    );
    return screen_point;
}

Ray MouseRay(float width, float height, V3 camera_position, Basis camera_basis, V2 mouse) {
	V3 direction =
        (2.0 * mouse.x / width - 1.0) *    camera_basis.x +
        (height - 2.0 * mouse.y) / width * camera_basis.y + 
                                           camera_basis.z;
	return Ray(camera_position, direction);
}

Entity CreateEntity(
    const char* name, 
    Entity_type type,
    Mesh* mesh,
    bool active
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
            Log::Fatal("Invalid entity type.");
        }
    }

    return entity;
}

Entity* AddEntity(Free_List<Entity>& entities, Entity entity) {
    uint32 index = entities.Insert(entity);
    Entity* result = &entities[index];
    return result;
}
