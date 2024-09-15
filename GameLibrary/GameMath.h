#ifndef GAME_MATH
#define GAME_MATH

#pragma once
#include "math.h"

// Utility macros
#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) ((uint64)Megabytes(Value)*1024)

// Platform independent constants
static double Pi = 3.14159265359;
static double Tau = 2.0 * Pi;
static double twroot = 1.05946309436;

// Arithmetics
inline int32 CustomRound(double X) {
	return X >= 0 ? (int32)(X + 0.5f) : (int32)(X - 0.5f);
}

inline int32 Sign(double x) {
	if (x > 0) {
		return 1;
	}
	else if (x < 0) {
		return -1;
	}
	else {
		return 0;
	}
}

// Vectors
// 2D
struct v2 {
	double X, Y;
};

inline v2 V2(double X, double Y) {
	v2 Result;
	Result.X = X;
	Result.Y = Y;
	return Result;
}

inline v2 operator+(v2 A, v2 B) {
	v2 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	return Result;
}

inline v2 operator-(v2 A) {
	v2 Result;
	Result.X = -A.X;
	Result.Y = -A.Y;
	return Result;
}

inline v2 operator-(v2 A, v2 B) {
	v2 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	return Result;
}

inline v2 operator*(double C, v2 A) {
	v2 Result;
	Result.X = C * A.X;
	Result.Y = C * A.Y;
	return Result;
}

inline double operator*(v2 A, v2 B) {
	return A.X * B.X + A.Y * B.Y;
}

inline double module(v2 A) {
	return sqrt(A * A);
}

inline v2 normalize(v2 V) {
	return (module(V) < 0.00001f) ? V : (1 / module(V)) * V;
}

inline v2 project(v2 A, v2 B) {
	v2 N = normalize(B);
	return (A * N) * N;
}
 
inline v2 perp(v2 A) {
	return { -A.Y, A.X };
}

// 3D
struct v3 {
	double X, Y, Z;
};

inline v3 V3(double X, double Y, double Z) {
	v3 Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	return Result;
}

inline v3 operator+(v3 A, v3 B) {
	v3 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	Result.Z = A.Z + B.Z;
	return Result;
}

inline v3 operator-(v3 A) {
	v3 Result;
	Result.X = -A.X;
	Result.Y = -A.Y;
	Result.Z = -A.Z;
	return Result;
}

inline v3 operator-(v3 A, v3 B) {
	v3 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	Result.Z = A.Z - B.Z;
	return Result;
}

inline v3 operator*(double C, v3 A) {
	v3 Result;
	Result.X = C * A.X;
	Result.Y = C * A.Y;
	Result.Z = C * A.Z;
	return Result;
}

 inline v3 operator*(v3 A, double C) {
	v3 Result;
	Result.X = C * A.X;
	Result.Y = C * A.Y;
	Result.Z = C * A.Z;
	return Result;
}

 inline double operator*(v3 A, v3 B) {
	 return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
 }

inline v3& operator+=(v3& A, v3 B) {
	A.X += B.X;
	A.Y += B.Y;
	A.Z += B.Z;
	return A;
}

inline v3& operator-=(v3& A, v3 B) {
	A.X -= B.X;
	A.Y -= B.Y;
	A.Z -= B.Z;
	return A;
}

inline v3& operator*=(v3& A, double C) {
	A.X *= C;
	A.Y *= C;
	A.Z *= C;
	return A;
}

inline v3 cross(v3 A, v3 B) {
	v3 Result;
	Result.X = A.Y * B.Z - A.Z * B.Y;
	Result.Y = A.Z * B.X - A.X * B.Z;
	Result.Z = A.X * B.Y - A.Y * B.X;
	return Result;
}

inline double module(v3 A) {
	return sqrt(A*A);
}

inline v3 normalize(v3 V) {
	return (module(V) < 0.00001f) ? V : (1 / module(V)) * V;
}

inline v3 project(v3 A, v3 B) {
	v3 N = normalize(B);
	return (A * N) * N;
}

inline v3 DirectionFromAngle(double Theta) {
	v3 Result;
	Result.X = cos(Theta);
	Result.Y = sin(Theta);
	Result.Z = 0;
	return Result;
}

inline v3 SphereNormal(double Theta, double Phi) {
	v3 Result;
	Result.X = cos(Phi)*sin(Theta);
	Result.Y = sin(Phi)*sin(Theta);
	Result.Z = cos(Theta);
	return Result;
}

inline v3 Rotate(v3 v, double Angle) {
	double X = v.X * cos(Angle) + v.Y * sin(Angle);
	double Y = -v.X * sin(Angle) + v.Y * cos(Angle);
	double Z = v.Z;
	return V3(X, Y, Z);
}

inline v3 Rotate(v3 v, v3 w, double Angle = 0.0) {
	if (Angle == 0.0) {
		double Angle = Tau * module(w);
	}
	v3 n = normalize(w);
	// Rodrigues formula
	return cos(Angle) * v + sin(Angle) * cross(n, v) + (1 - cos(Angle)) * (n * v) * n;
}


typedef double matrix3[9];
typedef double matrix4[16];


struct basis {
	v3 X;
	v3 Y;
	v3 Z;
};

inline v3 ChangeBasis(v3 V, basis Basis) {
	return V.X * Basis.X + V.Y * Basis.Y + V.Z * Basis.Z;
}

inline basis Rotate(basis Basis, double Angle) {
	basis Result;
	Result.X = Rotate(Basis.X, Angle);
	Result.Y = Rotate(Basis.Y, Angle);
	Result.Z = Rotate(Basis.Z, Angle);
	return Result;
}

inline basis Rotate(basis Basis, v3 w) {
	return {
		Rotate(Basis.X, w),
		Rotate(Basis.Y, w),
		Rotate(Basis.Z, w)
	};
}

struct scale {
	double X;
	double Y;
	double Z;
};

inline scale Scale(double X = 1.0, double Y = 1.0, double Z = 1.0) {
	return { X, Y, Z };
};

inline v3 operator*(scale Scale, v3 Vector) {
	return V3(Scale.X * Vector.X, Scale.Y * Vector.Y, Scale.Z * Vector.Z);
}

inline basis operator*(scale Scale, basis Basis) {
	return { Scale.X * Basis.X, Scale.Y * Basis.Y, Scale.Z * Basis.Z };
}

inline basis operator*(double Factor, basis Basis) {
	return { Factor * Basis.X, Factor * Basis.Y, Factor * Basis.Z };
}

inline basis Identity(double Factor = 1.0) {
	return { V3(Factor, 0.0, 0.0), V3(0.0, Factor, 0.0), V3(0.0, 0.0, Factor) };
}

inline basis normalize(basis Basis) {
	return {
		normalize(Basis.X),
		normalize(Basis.Y),
		normalize(Basis.Z)
	};
}

struct quaternion {
	double c;
	double i;
	double j;
	double k;
};

const quaternion I = { 0.0, 1.0, 0.0, 0.0 };
const quaternion J = { 0.0, 0.0, 1.0, 0.0 };
const quaternion K = { 0.0, 0.0, 0.0, 1.0 };

inline quaternion Quaternion(double c, double i = 0.0, double j = 0.0, double k = 0.0) {
	return { c, i, j, k };
}

inline quaternion Quaternion(double Angle, v3 Vector) {
	double sin_angle = sin(Angle / 2);
	return { cos(Angle / 2.0), sin_angle * Vector.X, sin_angle * Vector.Y, sin_angle * Vector.Z};
}

inline quaternion Conjugate(quaternion q) {
	return { q.c, -q.i, -q.j, -q.k};
}

inline quaternion operator*(quaternion q1, quaternion q2) {
	return { 
		q1.c * q2.c - q1.i * q2.i - q1.j * q2.j - q1.k * q2.k,
		q1.c * q2.i + q1.i * q2.c + q1.j * q2.k - q1.k * q2.j,
		q1.c * q2.j + q1.j * q2.c + q1.k * q2.i - q1.i * q2.k,
		q1.c * q2.k + q1.k * q2.c + q1.i * q2.j - q1.j * q2.i,
	};
}

inline quaternion operator*(double c, quaternion q) {
	return { c * q.c, c * q.i, c * q.j, c * q.k};
}

inline quaternion operator*(quaternion q, double c) {
	return { c * q.c, c * q.i, c * q.j, c * q.k };
}

inline quaternion operator+(quaternion q1, quaternion q2) {
	return {q1.c + q2.c, q1.i + q2.i, q1.j + q2.j, q1.k + q2.k };
}

inline quaternion operator-(quaternion q1, quaternion q2) {
	return { q1.c - q2.c, q1.i - q2.i, q1.j - q2.j, q1.k - q2.k };
}

inline quaternion operator-(quaternion q) {
	return { -q.c, -q.i, -q.j, -q.k };
}

inline v3 Rotate(v3 Vector, quaternion Q) {
	double S = Q.c;
	v3 U = V3(Q.i, Q.j, Q.k);
	return 2.0 * ((U * Vector) * U + S * cross(U, Vector)) + (S*S - U * U) * Vector;
}

inline basis Rotate(basis Basis, quaternion Q) {
	return {
		Rotate(Basis.X, Q),
		Rotate(Basis.Y, Q),
		Rotate(Basis.Z, Q)
	};
}


// Transform
struct transform {
	v3 Translation;
	scale Scale;
	quaternion Rotation;
};

inline transform Transform(quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0), v3 Translation = V3(0.0, 0.0, 0.0), scale Scaling = Scale()) {
	return { Translation, Scaling, Rotation };
}

inline v3 operator*(transform Transform, v3 Vector) {
	return Rotate((Transform.Scale * Vector), Transform.Rotation) + Transform.Translation;
}

inline void Matrix(matrix4 Matrix, transform Transform) {
	Matrix[0]  = 2.0 * Transform.Scale.X * (Transform.Rotation.c * Transform.Rotation.c + Transform.Rotation.i * Transform.Rotation.i) - 1.0;
	Matrix[1]  = 2.0 * (Transform.Rotation.i * Transform.Rotation.j + Transform.Rotation.c * Transform.Rotation.k);
	Matrix[2]  = 2.0 * (Transform.Rotation.i * Transform.Rotation.k - Transform.Rotation.c * Transform.Rotation.j);
	Matrix[3]  = Transform.Translation.X;
	Matrix[4]  = 2.0 * (Transform.Rotation.i * Transform.Rotation.j - Transform.Rotation.c * Transform.Rotation.k);
	Matrix[5]  = 2.0 * Transform.Scale.Y * (Transform.Rotation.c * Transform.Rotation.c + Transform.Rotation.j * Transform.Rotation.j) - 1.0;
	Matrix[6]  = 2.0 * (Transform.Rotation.j * Transform.Rotation.k + Transform.Rotation.c * Transform.Rotation.i);
	Matrix[7]  = Transform.Translation.Y;
	Matrix[8]  = 2.0 * (Transform.Rotation.i * Transform.Rotation.k + Transform.Rotation.c * Transform.Rotation.j);
	Matrix[9]  = 2.0 * (Transform.Rotation.j * Transform.Rotation.k - Transform.Rotation.c * Transform.Rotation.i);
	Matrix[10] = 2.0 * Transform.Scale.Z * (Transform.Rotation.c * Transform.Rotation.c + Transform.Rotation.k * Transform.Rotation.k) - 1.0;
	Matrix[11] = Transform.Translation.Z;
	Matrix[12] = 0.0;
	Matrix[13] = 0.0;
	Matrix[14] = 0.0;
	Matrix[15] = 1.0;
}

// Geometry
struct game_triangle {
	v3 Points[3];
};

struct game_rect {
	double Left;
	double Top;
	double Width;
	double Height;
};

#endif