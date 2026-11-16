#include "pch.h"
#include "geometry.h"


Line2::Line2(V2 p, V2 d) : point(p), direction(Normalize(d)) {}

Line2 Line2::FromPoints(V2 p1, V2 p2) {
	return Line2(p1, p2 - p1);
}

bool AreParallel(Line2 lhs, Line2 rhs) {
	return AreParallel(lhs.direction, rhs.direction);
}

Line3::Line3(V3 p, V3 d) : point(p), direction(Normalize(d)) {}

Line3 LineFromPoints(V3 p1, V3 p2) {
	return Line3(p1, Normalize(p2 - p1));
}

V3 ClosestPoint(Line3 line, V3 point) {
	float t = Dot(line.direction, point - line.point);
	return point + t * line.direction;
}

float Distance(Line3 line, V3 point) {
	return Dot(line.direction, point - line.point);
}

float Distance(Line3 line1, Line3 line2) {
	float a = Dot(line1.direction, line2.direction);
	if (a == 0.0) { // Lines are parallel
		return Distance(line1, line2.point);
	}
	else {
		V3 v = line1.point - line2.point;
		float b1 = Dot(line1.direction, v);
		float b2 = Dot(line2.direction, v);
		float s = (a * b2 - b1) / a;
		float t = (b2 - a * b1) / a;
		V3 point1 = line1.point + s * line1.point;
		V3 point2 = line2.point + t * line2.direction;
		return Modulus(point1 - point2);
	}
}

Ray::Ray(V3 p, V3 d) : point(p), direction(Normalize(d)) {}

V3 ClosestPoint(Ray ray, V3 point) {
	float t = Dot(ray.direction, point - ray.point);
	if (t <= 0.0f) return ray.point;
	else return ray.point + t * ray.direction;
}

float SqDistance(Ray ray, V3 point) {
	V3 D = point - ClosestPoint(ray, point);
	return Dot(D,D);
}

Segment2::Segment2(V2 h, V2 t) : head(h), tail(t) {}

bool AreParallel(Segment2 s1, Segment2 s2) {
	return AreParallel(s1.tail - s1.head, s2.tail - s2.head);
}

bool IsInside(Segment2 s, V2 p) {
	V2 u = s.head - p;
	V2 v = s.tail - p;

	return AreParallel(u,v) && Dot(u,v) <= 0;
}

/*
	Intersects two 2D segments. If result is 0, there is no intersection. If result is 1, the intersection is exactly one point,
	which will be given in the `IntersectionPoint` parameter. If result is 2, the two segments overlap over another segment, which will
	be given in the `IntersectionSegment` parameter.
*/
int Intersect(Segment2 s1, Segment2 s2, V2& intersection_point, Segment2& intersection_segment) {
	if (AreParallel(s1, s2)) {
		V2 u = s1.tail - s1.head;
		V2 v = s2.head - s1.head;
		if (AreParallel(u, v)) {
			float r0 = Dot(u, u);                 // Coordinates for S1.Tail
			float r1 = Dot(u, v);                 // Coordinates for S2.Head
			float r2 = Dot(u, s2.tail - s1.head); // Coordinates for S2.Tail

			// Swap if segments point in opposite directions
			bool swap = r2 - r1 < 0;
			if (swap) {
				float temp = r1;
				r1 = r2;
				r2 = temp;
				V2 temp2 = s2.head;
				s2.head = s2.tail;
				s2.tail = temp2;
			}

			if (fabsf(r1) < EPSILON) {
				if (r2 < EPSILON) {
					intersection_point = s2.head;
					return 1;
				}
				else {
					intersection_segment = s2;
					if (r2 > r0) {
						intersection_segment.tail = s1.tail;
					}
					return 2;
				}
			}
			else if (r1 < 0) {
				if (fabsf(r2) < EPSILON) {
					intersection_point = s2.tail;
					return 1;
				}
				else if (r2 < 0) {
					return 0;
				}
				else {
					intersection_segment = s1;
					if (r2 < r0) {
						intersection_segment.tail = s2.tail;
					}
					return 2;
				}
			}
			else {
				if (fabsf(r1 - r0) < EPSILON) {
					intersection_point = s2.head;
					return 1;
				}
				else if (r1 > r0) {
					return 0;
				}
				else {
					intersection_segment.head = s2.head;
					intersection_segment.tail = s1.tail;
					return 2;
				}
			}
		}
		return 0;
	}
	else {
		V2 a = s1.head;
		V2 b = s1.tail;
		V2 c = s2.head;
		V2 d = s2.tail;

		float orient1 = Cross(b - a, c - a);
		float orient2 = Cross(b - a, d - a);
		float orient3 = Cross(d - c, a - c);
		float orient4 = Cross(d - c, b - c);

		if (orient1 * orient2 > 0 || orient3 * orient4 > 0) {
			return 0;
		}
		else {
			float t = - orient1 / Cross(b - a, d - c);
			assert(t >= 0 && t <= 1);
			intersection_point = c + t * (d - c);
			return 1;
		}
	}
	return 0;
}

V2 ClosestPoint(Segment2 segment, V2 point) {
	V2 v = segment.tail - segment.head;

	float t = Dot(v, point);
	if (t <= 0.0f) return segment.head;
	float denom = Dot(v, v);
	if (t >= denom) return segment.tail;
	else return segment.head + (t / denom) * v;
}

float SqDistance(Segment2 segment, V2 point) {
	V2 v = segment.tail - segment.head;
	V2 r = point - segment.head;

	r = Project(r, v) - point;
	return Dot(r, r);
}

float SqDistance(Segment2 segment1, Segment2 segment2) {
	float d1 = SqDistance(segment1, segment2.head);
	float d2 = SqDistance(segment1, segment2.tail);
	float d3 = SqDistance(segment2, segment1.head);
	float d4 = SqDistance(segment2, segment2.tail);

	return fmin(d1, fmin(d2, fmin(d3, d4)));
}

Segment3 operator*(Transform transform, Segment3 segment) {
	return Segment3(transform * segment.head, transform * segment.tail);
}

/*
	Returns the transform that moves the segment [origin, (0,L,0)] to the input segment, L being the length of this segment.
*/
Transform SegmentTransform(Segment3 segment) {
	V3 translation = segment.head;
	V3 v = segment.tail - segment.head;
	float l = Modulus(v);
	float angle = acosf(v.y / l);
	Quaternion rotation = Quaternion(angle, V3(-v.z, 0, v.x));
	return Transform(translation, rotation, Scale(1, 1, 1));
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
	V3 v = ray.point - segment.head;

    float b = Dot(ray.direction, segment_direction);
    float c = Dot(segment_direction, segment_direction);
    float d = Dot(ray.direction, v);
    float e = Dot(segment_direction, v);

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

    V3 w = v + t * ray.direction - u * segment_direction;
	return Dot(w, w);
}

Triangle2 Flip(Triangle2 t) {
	return { t.points[0], t.points[2], t.points[1] };
}

Triangle2 Cycle(Triangle2 t) {
	return { t.points[1], t.points[2], t.points[0] };
}

bool operator==(Triangle2 lhs, Triangle2 rhs) {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 3; j++) {
			if (
				lhs.points[0] == rhs.points[0] && 
				lhs.points[1] == rhs.points[1] && 
				lhs.points[2] == rhs.points[2]
			) {
				return true;
			}
			rhs = Cycle(rhs);
		}
		rhs = Flip(rhs);
	}
	return false;
}

float GetArea(Triangle2 t) {
	return 0.5f * Cross(t.points[2] - t.points[0], t.points[1] - t.points[0]);
}

bool IsInside(Triangle2 t, V2 p) {
	Triangle2 u = { p, t.points[1], t.points[2] };
	Triangle2 v = { p, t.points[2], t.points[0] };
	Triangle2 w = { p, t.points[0], t.points[1] };

	float area0 = GetArea(u);
	float area1 = GetArea(v);
	float area2 = GetArea(w);

	if (area0 * area1 > 0 && area1 * area2 > 0) {
		return true;
	}

	Segment2 s1 = { t.points[0], t.points[1] };
	Segment2 s2 = { t.points[1], t.points[2] };
	Segment2 s3 = { t.points[2], t.points[0] };

	return IsInside(s1, p) || IsInside(s2, p) || IsInside(s3, p);
}

float SqDistance(Triangle2 t, V2 p) {
	Segment2 e[3] = {
		{ t.points[0], t.points[1] },
		{ t.points[1], t.points[2] },
		{ t.points[2], t.points[0] },
	};

	float d = FLT_MAX;
	for (int i = 0; i < 3; i++) {
		float new_d = SqDistance(e[i], p);
		if (new_d < d) {
			d = new_d;
		}
	}

	return d;
}


/*
	Intersects two triangles in the plane. result is true if both triangles intersect in a region with some area.
*/
bool Intersect(Triangle2 t1, Triangle2 t2) {
	float a1 = GetArea(t1);
	if (fabsf(a1) < EPSILON) return false;

	float a2 = GetArea(t2);
	if (fabsf(a2) < EPSILON) return false;

	// Apply separating hiperplane theorem
	for (int i = 0; i < 6; i++) {
		V2 v;
		if (i < 3) {
			v = Perpendicular(t1.points[i] - t1.points[(i+1)%3]);
		}
		else {
			v = Perpendicular(t2.points[i-3] - t2.points[(i-2)%3]);
		}

		float c1[3] = {};
		float c2[3] = {};

		for (int j = 0; j < 3; j++) {
			c1[j] = Dot(v, t1.points[j]);
			c2[j] = Dot(v, t2.points[j]);
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
	V2 c1 = Center(rect1);
	V2 c2 = Center(rect2);
	return fabsf(c1.x - c2.x) < 0.5f * (rect1.width  + rect2.width) && 
	       fabsf(c1.y - c2.y) < 0.5f * (rect1.height + rect2.height);
}

Vector_Plane::Vector_Plane(V3 n) : normal(Normalize(n)) {}

Affine_Plane::Affine_Plane(V3 point, V3 n) : normal(Normalize(n)), c(Dot(normal, point)) {}

V3 ClosestPoint(Affine_Plane plane, V3 point) {
	return point - (Dot(point, plane.normal) - plane.c) * plane.normal;
}

float Distance(Affine_Plane plane, V3 point) {
	return Dot(point, plane.normal) - plane.c;
}

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Colliders                                                                                                                                    |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/

Collider RectCollider(V2 offset, float width, float height) {
	Collider result = {
		.type = Rect_Collider,
		.offset = V3(offset.x, offset.y, 0)
	};
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
	Collider result = {
		.type = Cube_Collider,
		.offset = offset
	};
	result.cube.half_width = 0.5f * width;
	result.cube.half_height = 0.5f * height;
	result.cube.half_depth = 0.5f * depth;
	return result;
}

Collider SphereCollider(V3 offset, float radius) {
	Collider result = {
		.type = Sphere_Collider,
		.offset = offset
	};
	result.sphere.radius = radius;
	return result;
}

Collider CapsuleCollider(V3 head, V3 tail, float distance) {
	Collider result = {
		.type = Capsule_Collider,
		.offset = 0.5f * (head + tail)
	};
	result.capsule.segment = {head, tail};
	result.capsule.distance = distance;
	return result;
}

Collider operator*(Transform t, Collider c) {
	switch(c.type) {
        case Rect_Collider: {
            V3 displacement = t.translation;
			displacement.z = 0;
			c.offset += displacement;
			return c;
        } break;

		case Sphere_Collider:
        case Cube_Collider: {
            V3 displacement = t.translation;
			c.offset += displacement;
			return c;
        } break;

		case Capsule_Collider: {
			c.capsule.segment = t * c.capsule.segment;
			return c;
		} break;

		default: Log::Fatal("Invalid collider type.");
	}
	return c;
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
				float sum_radii = collider1.sphere.radius + collider2.sphere.radius;
				V3 r = collider2.offset - collider1.offset;
				return Dot(r, r) <= sum_radii * sum_radii;
			} break;
			case Capsule_Collider: {
				float sum_distance = collider1.capsule.distance + collider1.capsule.distance;
				return SqDistance(collider1.capsule.segment, collider2.capsule.segment) <= sum_distance * sum_distance;
			} break;
			default: Log::Fatal("Invalid collider type");
		}
	}
	else {
		if (collider1.type > collider2.type) {
			Collider c = collider2;
			collider2 = collider1;
			collider1 = c;
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
				float sum_distances = collider1.sphere.radius + collider2.capsule.distance;
				return SqDistance(collider2.capsule.segment, collider1.offset) <= sum_distances * sum_distances;
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