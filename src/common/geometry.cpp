#include "pch.h"
#include "geometry.h"


Line2::Line2(V2 p, V2 d) : point(p), direction(Normalize(d)) {}

Line2 Line2::FromPoints(V2 p1, V2 p2) {
	return Line2(p1, p2 - p1);
}

bool AreParallel(Line2 lhs, Line2 rhs) {
	return AreParallel(lhs.direction, rhs.direction);
}

Line3 Line(V3 Point, V3 Direction) {
	return { Point, Normalize(Direction) };
}

Line3 LineFromPoints(V3 A, V3 B) {
	return { A, Normalize(B - A) };
}

V3 ClosestPoint(Line3 Line, V3 Point) {
	float t = Dot(Line.Direction, Point - Line.Point);
	return Point + t * Line.Direction;
}

float Distance(Line3 Line, V3 Point) {
	return Dot(Line.Direction, Point - Line.Point);
}

float Distance(Line3 Line1, Line3 Line2) {
	float a = Dot(Line1.Direction, Line2.Direction);
	if (a == 0.0) { // Lines are parallel
		return Distance(Line1, Line2.Point);
	}
	else {
		V3 v = Line1.Point - Line2.Point;
		float b1 = Dot(Line1.Direction, v);
		float b2 = Dot(Line2.Direction, v);
		float s = (a * b2 - b1) / a;
		float t = (b2 - a * b1) / a;
		V3 Point1 = Line1.Point + s * Line1.Point;
		V3 Point2 = Line2.Point + t * Line2.Direction;
		return modulus(Point1 - Point2);
	}
}

Ray BuildRay(V3 point, V3 direction) {
	return { point, Normalize(direction) };
}

V3 ClosestPoint(Ray ray, V3 point) {
	float t = Dot(ray.direction, point - ray.point);
	if (t <= 0.0f) return ray.point;
	else return ray.point + t * ray.direction;
}

float SqDistance(Ray ray, V3 point) {
	V3 D = point - ClosestPoint(ray, point);
	return Dot(D,D);
}

bool AreParallel(Segment2 S1, Segment2 S2) {
	return AreParallel(S1.tail - S1.head, S2.tail - S2.head);
}

bool IsInside(Segment2 S, V2 P) {
	V2 u = S.head - P;
	V2 v = S.tail - P;

	return AreParallel(u,v) && Dot(u,v) <= 0;
}

/*
	Intersects two 2D segments. If result is 0, there is no intersection. If result is 1, the intersection is exactly one point,
	which will be given in the `IntersectionPoint` parameter. If result is 2, the two segments overlap over another segment, which will
	be given in the `IntersectionSegment` parameter.
*/
int Intersect(Segment2 S1, Segment2 S2, V2* IntersectionPoint, Segment2* IntersectionSegment) {
	if (AreParallel(S1, S2)) {
		V2 u = S1.tail - S1.head;
		V2 v = S2.head - S1.head;
		if (AreParallel(u, v)) {
			float R0 = Dot(u, u);                 // Coordinates for S1.Tail
			float R1 = Dot(u, v);                 // Coordinates for S2.Head
			float R2 = Dot(u, S2.tail - S1.head); // Coordinates for S2.Tail

			// Swap if segments point in opposite directions
			bool Swap = R2 - R1 < 0;
			if (Swap) {
				float Temp = R1;
				R1 = R2;
				R2 = Temp;
				V2 Temp2 = S2.head;
				S2.head = S2.tail;
				S2.tail = Temp2;
			}

			if (fabsf(R1) < EPSILON) {
				if (R2 < EPSILON) {
					*IntersectionPoint = S2.head;
					return 1;
				}
				else {
					*IntersectionSegment = S2;
					if (R2 > R0) {
						IntersectionSegment->tail = S1.tail;
					}
					return 2;
				}
			}
			else if (R1 < 0) {
				if (fabsf(R2) < EPSILON) {
					*IntersectionPoint = S2.tail;
					return 1;
				}
				else if (R2 < 0) {
					return 0;
				}
				else {
					*IntersectionSegment = S1;
					if (R2 < R0) {
						IntersectionSegment->tail = S2.tail;
					}
					return 2;
				}
			}
			else {
				if (fabsf(R1 - R0) < EPSILON) {
					*IntersectionPoint = S2.head;
					return 1;
				}
				else if (R1 > R0) {
					return 0;
				}
				else {
					IntersectionSegment->head = S2.head;
					IntersectionSegment->tail = S1.tail;
					return 2;
				}
			}
		}
		return 0;
	}
	else {
		V2 A = S1.head;
		V2 B = S1.tail;
		V2 C = S2.head;
		V2 D = S2.tail;

		float Orient1 = Cross(B - A, C - A);
		float Orient2 = Cross(B - A, D - A);
		float Orient3 = Cross(D - C, A - C);
		float Orient4 = Cross(D - C, B - C);

		if (Orient1 * Orient2 > 0 || Orient3 * Orient4 > 0) {
			return 0;
		}
		else {
			float t = - Orient1 / Cross(B - A, D - C);
			assert(t >= 0 && t <= 1);
			*IntersectionPoint = C + t * (D - C);
			return 1;
		}
	}
	return 0;
}

V2 ClosestPoint(Segment2 Segment, V2 Point) {
	V2 V = Segment.tail - Segment.head;

	float t = Dot(V, Point);
	if (t <= 0.0f) return Segment.head;
	float Denom = Dot(V, V);
	if (t >= Denom) return Segment.tail;
	else return Segment.head + (t / Denom) * V;
}

float SqDistance(Segment2 Segment, V2 Point) {
	V2 V = Segment.tail - Segment.head;
	V2 R = Point - Segment.head;

	R = Project(R, V) - Point;
	return Dot(R,R);
}

float SqDistance(Segment2 Segment1, Segment2 Segment2) {
	float d1 = SqDistance(Segment1, Segment2.head);
	float d2 = SqDistance(Segment1, Segment2.tail);
	float d3 = SqDistance(Segment2, Segment1.head);
	float d4 = SqDistance(Segment2, Segment2.tail);

	return fmin(d1, fmin(d2, fmin(d3, d4)));
}

INTROSPECT
struct Segment3 {
	V3 head;
	V3 tail;
};

Segment3 operator*(Transform transform, Segment3 segment) {
	return { transform * segment.head, transform * segment.tail };
}

/*
	Returns the transform that moves the segment [origin, (0,L,0)] to the input segment, L being the length of this segment.
*/
Transform SegmentTransform(Segment3 segment) {
	Transform result;
	result.scale = Scale();
	result.translation = segment.head;
	V3 V = segment.tail - segment.head;
	float L = Modulus(V);
	float angle = acosf(V.y / L);
	result.rotation = Quaternion(angle, V3(-V.z,0,V.x));
	return result;
}

V3 ClosestPoint(Segment3 segment, V3 point) {
	V3 V = segment.tail - segment.head;

	float t = Dot(V, point);
	if (t <= 0.0f) return segment.head;
	float denom = Dot(V, V);
	if (t >= denom) return segment.tail;
	else return segment.head + (t / denom) * V;
}

float SqDistance(Segment3 segment, V3 point) {
	V3 ab = segment.tail - segment.head;
	V3 ac = point - segment.head;
	V3 bc = point - segment.tail;
	float e = Dot(ac, ab);
	// Handle cases where c Projects outside ab
	if (e <= 0.0f) return Dot(ac, ac);
	float f = Dot(ab, ab);
	if (e >= f) return Dot(bc, bc);
	// Handle cases where c Projects onto ab
	return Dot(ac, ac) - e * e / f;
}

float SqDistance(Segment3 segment1, Segment3 segment2) {
	V3 d1 = segment1.tail - segment1.head;
	V3 d2 = segment2.tail - segment2.head;
	V3 r = segment1.head - segment2.head;
	float a = Dot(d1, d1);
	float b = Dot(d1, d2);
	float c = Dot(d2, d2);
	float d = Dot(d1, r);
	float e = Dot(d2, r);
	float denom = a * c - b * b;
	float s = 0.0f;
	if (denom != 0.0f) s = Clamp((b * e - d * c) / denom, 0.0f, 1.0f);
	float t = (b * s + e) / c;
	if (t < 0.0f) {
		t = 0.0f;
		s = Clamp(-d / a, 0.0f, 1.0f);
	}
	else if (t > 1.0f) {
		t = 1.0f;
		s = Clamp((b - d) / a, 0.0f, 1.0f);
	}

	V3 p1 = segment1.head + s * d1;
	V3 p2 = segment2.head + t * d2;
	return Dot(p1 - p2, p1 - p2);
}

float SqDistance(Segment3 segment, Ray ray) {
	V3 segment_direction = segment.tail - segment.head;
	V3 V = ray.point - segment.head;

    float b = Dot(ray.direction, segment_direction);
    float c = Dot(segment_direction, segment_direction);
    float d = Dot(ray.direction, V);
    float e = Dot(segment_direction, V);

    float denom = c - b * b;
    float t, u;

    if (denom != 0.0f) {
        t = (b * e - c * d) / denom;
        u = (e - b * d) / denom;
    } else {
        // Lines are parallel: pick t = 0, Project S onto R
        t = 0.0f;
        u = e / c;
    }

    // Clamp u to [0,1] for the segment
    u = Clamp(u, 0.0f, 1.0f);

    // Clamp t to [0, inf) for the ray
    if (t < 0.0f) {
        t = 0.0f;
        // Recompute u for this t
        V3 P = ray.point + t * ray.direction;
        u = Dot(segment_direction, P - segment.head) / c;
        u = Clamp(u, 0.0f, 1.0f);
    }

    V3 D = V + t * ray.direction - u * segment_direction;
	return Dot(D, D);
}

struct triangle2 {
	V2 Points[3];
};

triangle2 Flip(triangle2 T) {
	return { T.Points[0], T.Points[2], T.Points[1] };
}

triangle2 Cycle(triangle2 T) {
	return { T.Points[1], T.Points[2], T.Points[0] };
}

bool operator==(triangle2 First, triangle2 Second) {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 3; j++) {
			if (
				First.Points[0] == Second.Points[0] && 
				First.Points[1] == Second.Points[1] && 
				First.Points[2] == Second.Points[2]
			) {
				return true;
			}
			Second = Cycle(Second);
		}
		Second = Flip(Second);
	}
	return false;
}

float GetArea(triangle2 T) {
	return 0.5f * Cross(T.Points[2] - T.Points[0], T.Points[1] - T.Points[0]);
}

bool IsInside(triangle2 T, V2 P) {
	triangle2 U = { P, T.Points[1], T.Points[2] };
	triangle2 V = { P, T.Points[2], T.Points[0] };
	triangle2 W = { P, T.Points[0], T.Points[1] };

	float Area0 = GetArea(U);
	float Area1 = GetArea(V);
	float Area2 = GetArea(W);

	if (Area0 * Area1 > 0 && Area1 * Area2 > 0) {
		return true;
	}

	Segment2 S1 = { T.Points[0], T.Points[1] };
	Segment2 S2 = { T.Points[1], T.Points[2] };
	Segment2 S3 = { T.Points[2], T.Points[0] };

	return IsInside(S1, P) || IsInside(S2, P) || IsInside(S3, P);
}

float SqDistance(triangle2 T, V2 P) {
	Segment2 E[3] = {
		{ T.Points[0], T.Points[1] },
		{ T.Points[1], T.Points[2] },
		{ T.Points[2], T.Points[0] },
	};

	float d = FLT_MAX;
	for (int i = 0; i < 3; i++) {
		float new_d = SqDistance(E[i], P);
		if (new_d < d) {
			d = new_d;
		}
	}

	return d;
}


/*
	Intersects two triangles in the plane. result is true if both triangles intersect in a region with some area.
*/
bool Intersect(triangle2 T1, triangle2 T2) {
	float A1 = GetArea(T1);
	if (fabsf(A1) < EPSILON) return false;

	float A2 = GetArea(T2);
	if (fabsf(A2) < EPSILON) return false;

	// Apply separating hiperplane theorem
	for (int i = 0; i < 6; i++) {
		V2 V = {};
		if (i < 3) {
			V = Perpendicular(T1.Points[i] - T1.Points[(i+1)%3]);
		}
		else {
			V = Perpendicular(T2.Points[i-3] - T2.Points[(i-2)%3]);
		}

		float c1[3] = {};
		float c2[3] = {};

		for (int j = 0; j < 3; j++) {
			c1[j] = Dot(V, T1.Points[j]);
			c2[j] = Dot(V, T2.Points[j]);
		}

		float d = 0;
		if (i < 3) d = c1[i];
		else       d = c2[i-3];
		for (int j = 0; j < 3; j++) {
			c1[j] -= d;
			if (fabsf(c1[j]) < 1) {
				c1[j] = 0;
			}
			c2[j] -= d;
			if (fabsf(c2[j]) < 1) {
				c2[j] = 0;
			}
		}

		if (
			c2[0] * c2[1] >= 0 && c2[1] * c2[2] >= 0 && c2[2] * c2[0] >= 0 &&
			c1[0] * c1[1] >= 0 && c1[1] * c1[2] >= 0 && c1[2] * c1[0] >= 0
		) {
			float OppositeVertex;
			float* c_arr;
			if (i < 3) { 
				OppositeVertex = c1[(i + 2) % 3];
				c_arr = c2;
			}
			else {
				OppositeVertex = c2[(i + 2) % 3];
				c_arr = c1;
			}
			if (OppositeVertex * c_arr[0] < 0 || OppositeVertex * c_arr[1] < 0 || OppositeVertex * c_arr[2] < 0) {
				return false;
			}
		}
	}

	return true;
}

struct triangle3 {
	V3 Points[3];
};

struct Rect {
	float left;
	float top;
	float width;
	float height;
};

bool IsIn(Rect rect, V2 position) {
	return rect.left <= position.x && position.x <= rect.left+rect.width &&
		   rect.top  <= position.y && position.y <= rect.top+rect.height;
}

V2 LeftTop(Rect rect) {
	return V2(rect.left, rect.top);
}

V2 Center(Rect rect) {
	return V2(rect.left + 0.5f * rect.width, rect.top  + 0.5f * rect.height);
}

bool Intersect(Rect rect1, Rect rect2) {
	V2 C1 = Center(rect1);
	V2 C2 = Center(rect2);
	return fabsf(C1.x - C2.x) < 0.5f * (rect1.width  + rect2.width) && 
	       fabsf(C1.y - C2.y) < 0.5f * (rect1.height + rect2.height);
}

struct vector_plane {
	V3 Normal;
};

vector_plane VectorPlane(V3 normal) {
	return { Normalize(normal) };
}

struct affine_plane {
	float c; // The equation of the plane is Dot(Normal, x) = c
	V3 Normal;
};

affine_plane AffinePlane(V3 Point, V3 Normal) {
	Normal = Normalize(Normal);
	return { Dot(Normal, Point), Normal };
}

V3 ClosestPoint(affine_plane Plane, V3 Point) {
	return Point - (Dot(Point, Plane.Normal) - Plane.c) * Plane.Normal;
}

float Distance(affine_plane Plane, V3 Point) {
	return Dot(Point, Plane.Normal) - Plane.c;
}

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Colliders                                                                                                                                    |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/

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

Collider RectCollider(V2 offset, float width, float height) {
	Collider result = {};
	result.type = Rect_Collider;
	result.offset = V3(offset.x, offset.y, 0);
	result.rect.half_width = 0.5f * width;
	result.rect.half_height = 0.5f * height;
	return result;
}

Rect GetRect(Collider collider) {
	assert(collider.type == Rect_Collider);
	return {
		collider.offset.x - collider.rect.half_width,
		collider.offset.y - collider.rect.half_height,
		2.0f * collider.rect.half_width,
		2.0f * collider.rect.half_height
	};
}

Collider CubeCollider(V3 offset, float width, float height, float depth) {
	Collider result = {};
	result.type = Cube_Collider;
	result.offset = offset;
	result.cube.half_width = 0.5f * width;
	result.cube.half_height = 0.5f * height;
	result.cube.half_depth = 0.5f * depth;
	return result;
}

Collider SphereCollider(V3 offset, float radius) {
	Collider result = {};
	result.type = Sphere_Collider;
	result.offset = offset;
	result.sphere.radius = radius;
	return result;
}

Collider CapsuleCollider(V3 head, V3 tail, float distance) {
	Collider result = {};
	result.type = Capsule_Collider;
	result.offset = 0.5f * (head + tail);
	result.capsule.segment = {head, tail};
	result.capsule.distance = distance;
	return result;
}

Collider operator*(Transform T, Collider C) {
	switch(C.type) {
        case Rect_Collider: {
            V3 Displacement = T.translation;
			Displacement.z = 0;
			C.offset += Displacement;
			return C;
        } break;

		case Sphere_Collider:
        case Cube_Collider: {
            V3 Displacement = T.translation;
			C.offset += Displacement;
			return C;
        } break;

		case Capsule_Collider: {
			C.capsule.segment = T * C.capsule.segment;
			return C;
		} break;

		default: Log::Fatal("Invalid collider type.");
	}
	return C;
}

bool Collide(Collider collider, V3 position) {
    switch(collider.type) {
        case Rect_Collider: {
            return fabsf(position.x - collider.offset.x) <= collider.rect.half_width &&
                   fabsf(position.y - collider.offset.y) <= collider.rect.half_height;
        } break;

        case Cube_Collider: {
            return fabsf(position.x - collider.offset.x) <= collider.cube.half_width &&
                   fabsf(position.y - collider.offset.y) <= collider.cube.half_height &&
                   fabsf(position.z - collider.offset.z) <= collider.cube.half_depth;
        } break;

        case Sphere_Collider: {
            return Dot(position - collider.offset, position - collider.offset) <= collider.sphere.radius * collider.sphere.radius;
        } break;

		case Capsule_Collider: {
			return SqDistance(collider.capsule.segment, position) <= collider.capsule.distance * collider.capsule.distance;
		} break;

		default: Log::Fatal("Invalid collider type.");
    }
    return false;
}

bool Collide(Collider collider1, Collider collider2) {
	if (collider1.type == collider2.type) {
		switch(collider1.type) {
			case Rect_Collider: {
				return Intersect(GetRect(collider1), GetRect(collider2));
			} break;
			case Cube_Collider: {
				float x[2] = { collider1.offset.x - collider1.cube.half_width,  collider1.offset.x + collider1.cube.half_width };
				float y[2] = { collider1.offset.y - collider1.cube.half_height, collider1.offset.y + collider1.cube.half_height };
				float z[2] = { collider1.offset.z - collider1.cube.half_depth,  collider1.offset.z + collider1.cube.half_depth };
				for (int i = 0; i < 2; i++) 
				for (int j = 0; j < 2; j++) 
				for (int k = 0; k < 2; k++) {
					if (Collide(collider2, V3(x[i], y[j], z[k]))) return true;
				}
				return false;
			} break;
			case Sphere_Collider: {
				float SumRadii = collider1.sphere.radius + collider2.sphere.radius;
				V3 r = collider2.offset - collider1.offset;
				return Dot(r, r) <= SumRadii * SumRadii;
			} break;
			case Capsule_Collider: {
				float SumDistance = collider1.capsule.distance + collider1.capsule.distance;
				return SqDistance(collider1.capsule.segment, collider2.capsule.segment) <= SumDistance * SumDistance;
			} break;
			default: Log::Fatal("Invalid collider type");
		}
	}
	else {
		if (collider1.type > collider2.type) {
			Collider C = collider2;
			collider2 = collider1;
			collider1 = C;
		}
		assert(collider1.type != Rect_Collider);
		switch (collider1.type) {
			case Cube_Collider: {
				Log::Fatal("Not implemented");
				if (collider2.type == Sphere_Collider) {

				}
				else if (collider2.type == Capsule_Collider) {

				}
			} break;

			case Sphere_Collider: {
				assert(collider2.type == Capsule_Collider);
				float SumDistances = collider1.sphere.radius + collider2.capsule.distance;
				return SqDistance(collider2.capsule.segment, collider1.offset) <= SumDistances * SumDistances;
			} break;

			default: Log::Fatal("Not implemented");
		}
	}
	return false;
}

/*
Fast Ray-Box Intersection
by Andrew Woo
from "Graphics Gems", Academic Press, 1990
*/
bool HitBoundingBox(float minB[3], float maxB[3], float origin[3], float dir[3], float coord[3])
/* double minB[NUMDIM], maxB[NUMDIM];		box */
/* double origin[NUMDIM], dir[NUMDIM];		ray */
/* double coord[NUMDIM];			hit point   */
{
    bool inside = true;
    char quadrant[3];
    int i;
    int whichPlane;
    float maxT[3];
    float candidatePlane[3];
    char LEFT = 1;
    char RIGHT = 0;
    char MIDDLE = 2;

    /* Find candidate planes; this loop can be avoided if
    rays cast all from the eye(assume perpsective view) */
    for (i = 0; i < 3; i++)
        if (origin[i] < minB[i]) {
            quadrant[i] = LEFT;
            candidatePlane[i] = minB[i];
            inside = false;
        }
        else if (origin[i] > maxB[i]) {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxB[i];
            inside = false;
        }
        else {
            quadrant[i] = MIDDLE;
        }

    /* Ray origin inside bounding box */
    if (inside) {
        coord = origin;
        return true;
    }


    /* Calculate T distances to candidate planes */
    for (i = 0; i < 3; i++)
        if (quadrant[i] != MIDDLE && dir[i] != 0.)
            maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
        else
            maxT[i] = -1.;

    /* Get largest of the maxT's for final choice of intersection */
    whichPlane = 0;
    for (i = 1; i < 3; i++)
        if (maxT[whichPlane] < maxT[i])
            whichPlane = i;

    /* Check final candidate actually inside box */
    if (maxT[whichPlane] < 0.) return false;
    for (i = 0; i < 3; i++)
        if (whichPlane != i) {
            coord[i] = origin[i] + maxT[whichPlane] * dir[i];
            if (coord[i] < minB[i] || coord[i] > maxB[i])
                return false;
        }
        else {
            coord[i] = candidatePlane[i];
        }
    return true;				/* ray hits box */
}

bool Raycast(Ray ray, Collider collider) {
	switch(collider.type) {
		case Rect_Collider: {
			return IsIn(GetRect(collider), V2(ray.point.x, ray.point.y));
		} break;
		case Cube_Collider: {
			float minB[3] = { 0 };
			minB[0] = collider.offset.x - collider.cube.half_width / 2.0f;
			minB[1] = collider.offset.y - collider.cube.half_height / 2.0f;
			minB[2] = collider.offset.z - collider.cube.half_depth / 2.0f;
			float maxB[3] = { 0 };
			maxB[0] = collider.offset.x + collider.cube.half_width / 2.0f;
			maxB[1] = collider.offset.y + collider.cube.half_height / 2.0f;
			maxB[2] = collider.offset.z + collider.cube.half_depth / 2.0f;
			float origin[3] = { ray.point.x, ray.point.y, ray.point.z };
			float dir[3] = { ray.direction.x, ray.direction.y, ray.direction.z };
			float coord[3] = { 0,0,0 };

			return HitBoundingBox(minB, maxB, origin, dir, coord);
		} break;
		case Sphere_Collider: {
			float R = collider.sphere.radius;
			return SqDistance(ray, collider.offset) < R*R;
		} break;
		case Capsule_Collider: {
			float R = collider.capsule.distance;
			return SqDistance(collider.capsule.segment, ray) < R*R;
		} break;
		default: Log::Fatal("Invalid collider type");
	}
	return false;
}