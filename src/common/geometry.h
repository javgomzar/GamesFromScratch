#pragma once

#include "mathematics.h"

struct Line2 {
	V2 point;
	V2 direction;
    Line2(V2 p, V2 d);
    static Line2 FromPoints(V2 p1, V2 p2);
};

bool AreParallel(Line2 lhs, Line2 rhs);

struct Line3 {
	V3 point;
	V3 direction;
	Line3(V3 p, V3 d);
	static Line3 FromPoints(V3 p1, V3 p2);
};

V3 ClosestPoint(Line3 line, V3 point);
float Distance(Line3 line, V3 point);
float Distance(Line3 line1, Line3 line2);

struct Ray {
	V3 point;
	V3 direction;
	Ray(V3 p, V3 d);
};

V3 ClosestPoint(Ray ray, V3 point);
float SqDistance(Ray ray, V3 point);

struct Segment2 {
	V2 head;
	V2 tail;
	Segment2(V2 h, V2 t);
};

bool AreParallel(Segment2 s1, Segment2 s2);
bool IsInside(Segment2 s, V2 p);
int Intersect(Segment2 s1, Segment2 s2, V2& intersection_point, Segment2& intersection_segment);
V2 ClosestPoint(Segment2 segment, V2 point);
float SqDistance(Segment2 segment, V2 point);
float SqDistance(Segment2 segment1, Segment2 segment2);

INTROSPECT
struct Segment3 {
	V3 head;
	V3 tail;
};

Segment3 operator*(Transform transform, Segment3 segment);
Transform SegmentTransform(Segment3 segment);
V3 ClosestPoint(Segment3 segment, V3 point);
float SqDistance(Segment3 segment, V3 point);
float SqDistance(Segment3 segment1, Segment3 segment2);
float SqDistance(Segment3 segment, Ray ray);

struct Triangle2 {
	V2 points[3];
};

Triangle2 Flip(Triangle2 t);
Triangle2 Cycle(Triangle2 t);
bool operator==(Triangle2 lhs, Triangle2 rhs);
float GetArea(Triangle2 t);
bool IsInside(Triangle2 t, V2 p);
float SqDistance(Triangle2 t, V2 p);
bool Intersect(Triangle2 t1, Triangle2 t2);

struct Triangle3 {
	V3 points[3];
};

struct Rect {
	float left;
	float top;
	float width;
	float height;
};

bool IsIn(Rect rect, V2 position);
V2 LeftTop(Rect rect);
V2 Center(Rect rect);
bool Intersect(Rect rect1, Rect rect2);

struct Vector_Plane {
	V3 normal;
	Vector_Plane(V3 normal);
};

struct Affine_Plane {
	V3 normal;
	float c; // The equation of the plane is Dot(normal, x) = c
	Affine_Plane(V3 point, V3 n);
};

V3 ClosestPoint(Affine_Plane plane, V3 point);
float Distance(Affine_Plane plane, V3 point);

enum Collider_Type {
    Rect_Collider,
    Cube_Collider,
    Sphere_Collider,
	Capsule_Collider
};

struct Collider {
    Collider_Type type;
    V3 offset;
    union {
		struct {
			float half_width;
			float half_height;
		} rect;
		struct {
			float half_width;
			float half_height;
			float half_depth;
		} cube;
		struct {
			float radius;
		} sphere;
		struct {
			Segment3 segment;
			float distance;
		} capsule;
	};
};

Collider RectCollider(V2 offset, float width, float height);
Rect GetRect(Collider collider);
Collider CubeCollider(V3 offset, float width, float height, float depth);
Collider SphereCollider(V3 offset, float radius);
Collider CapsuleCollider(V3 head, V3 tail, float distance);
Collider operator*(Transform t, Collider c);
bool Collide(Collider collider, V3 position);
bool Collide(Collider collider1, Collider collider2);
bool HitBoundingBox(float minB[3], float maxB[3], float origin[3], float dir[3], float coord[3]);
bool Raycast(Ray ray, Collider collider);
