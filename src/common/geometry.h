#pragma once

#include "mathematics.h"

struct Line2 {
	V2 point;
	V2 direction;
    Line2(V2 p, V2 d);
    static Line2 FromPoints(V2 p1, V2 p2);
};

struct Line3 {
	V3 point;
	V3 direction;
};

struct Ray {
	V3 point;
	V3 direction;
};

struct Segment2 {
	V2 head;
	V2 tail;
};
