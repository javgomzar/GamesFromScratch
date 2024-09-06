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
int32 CustomRound(double X) {
	return X >= 0 ? (int32)(X + 0.5f) : (int32)(X - 0.5f);
}

int32 Sign(double x) {
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

v2 V2(double X, double Y) {
	v2 Result;
	Result.X = X;
	Result.Y = Y;
	return Result;
}

v2 operator+(v2 A, v2 B) {
	v2 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	return Result;
}

v2 operator-(v2 A) {
	v2 Result;
	Result.X = -A.X;
	Result.Y = -A.Y;
	return Result;
}

v2 operator-(v2 A, v2 B) {
	v2 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	return Result;
}

v2 operator*(double C, v2 A) {
	v2 Result;
	Result.X = C * A.X;
	Result.Y = C * A.Y;
	return Result;
}

double operator*(v2 A, v2 B) {
	return A.X * B.X + A.Y * B.Y;
}

double module(v2 A) {
	return sqrt(A * A);
}

v2 normalize(v2 V) {
	return (module(V) < 0.00001f) ? V : (1 / module(V)) * V;
}

v2 project(v2 A, v2 B) {
	v2 N = normalize(B);
	return (A * N) * N;
}

v2 perp(v2 A) {
	return { -A.Y, A.X };
}

// 3D
struct v3 {
	double X, Y, Z;
};

v3 V3(double X, double Y, double Z) {
	v3 Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	return Result;
}

v3 operator+(v3 A, v3 B) {
	v3 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	Result.Z = A.Z + B.Z;
	return Result;
}

v3 operator-(v3 A) {
	v3 Result;
	Result.X = -A.X;
	Result.Y = -A.Y;
	Result.Z = -A.Z;
	return Result;
}

v3 operator-(v3 A, v3 B) {
	v3 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	Result.Z = A.Z - B.Z;
	return Result;
}

v3 operator*(double C, v3 A) {
	v3 Result;
	Result.X = C * A.X;
	Result.Y = C * A.Y;
	Result.Z = C * A.Z;
	return Result;
}

v3 operator*(v3 A, double C) {
	v3 Result;
	Result.X = C * A.X;
	Result.Y = C * A.Y;
	Result.Z = C * A.Z;
	return Result;
}

double operator*(v3 A, v3 B) {
	return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
}

v3 cross(v3 A, v3 B) {
	v3 Result;
	Result.X = A.Y * B.Z - A.Z * B.Y;
	Result.Y = A.Z * B.X - A.X * B.Z;
	Result.Z = A.X * B.Y - A.Y * B.X;
	return Result;
}

double module(v3 A) {
	return sqrt(A*A);
}

v3 normalize(v3 V) {
	return (module(V) < 0.00001f) ? V : (1 / module(V)) * V;
}

v3 project(v3 A, v3 B) {
	v3 N = normalize(B);
	return (A * N) * N;
}

v3 DirectionFromAngle(double Theta) {
	v3 Result;
	Result.X = cos(Theta);
	Result.Y = sin(Theta);
	Result.Z = 0;
	return Result;
}

v3 SphereNormal(double Theta, double Phi) {
	v3 Result;
	Result.X = cos(Phi)*sin(Theta);
	Result.Y = sin(Phi)*sin(Theta);
	Result.Z = cos(Theta);
	return Result;
}

v3 Rotate(v3 v, double Angle) {
	double X = v.X * cos(Angle) + v.Y * sin(Angle);
	double Y = -v.X * sin(Angle) + v.Y * cos(Angle);
	double Z = v.Z;
	return V3(X, Y, Z);
}

v3 Rotate(v3 v, v3 w, double Angle = 0.0) {
	if (Angle == 0.0) {
		double Angle = Tau * module(w);
	}
	v3 n = normalize(w);
	// Rodrigues formula
	return cos(Angle) * v + sin(Angle) * cross(n, v) + (1 - cos(Angle)) * (n * v) * n;
}

struct basis {
	v3 X;
	v3 Y;
	v3 Z;
};

v3 ChangeBasis(v3 V, basis Basis) {
	return V.X * Basis.X + V.Y * Basis.Y + V.Z * Basis.Z;
}

basis Rotate(basis Basis, double Angle) {
	basis Result;
	Result.X = Rotate(Basis.X, Angle);
	Result.Y = Rotate(Basis.Y, Angle);
	Result.Z = Rotate(Basis.Z, Angle);
	return Result;
}

basis Rotate(basis Basis, v3 w) {
	return {
		Rotate(Basis.X, w),
		Rotate(Basis.Y, w),
		Rotate(Basis.Z, w)
	};
}

basis Scale(basis Basis, double ScaleX, double ScaleY, double ScaleZ) {
	basis Result;
	Result.X = ScaleX * Basis.X;
	Result.Y = ScaleY * Basis.Y;
	Result.Z = ScaleZ * Basis.Z;
	return Result;
}

basis Scale(basis Basis, double Factor) {
	return Scale(Basis, Factor, Factor, Factor);
}

basis Identity(double Scale = 1.0) {
	return { V3(Scale, 0.0, 0.0), V3(0.0, Scale, 0.0), V3(0.0, 0.0, Scale) };
}

basis normalize(basis Basis) {
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

quaternion Quaternion(double c, double i = 0.0, double j = 0.0, double k = 0.0) {
	return { c, i, j, k };
}

quaternion Quaternion(double Angle, v3 Vector) {
	double sin_angle = sin(Angle / 2);
	return { cos(Angle / 2.0), sin_angle * Vector.X, sin_angle * Vector.Y, sin_angle * Vector.Z};
}

quaternion Conjugate(quaternion q) {
	return { q.c, -q.i, -q.j, -q.k};
}

quaternion operator*(quaternion q1, quaternion q2) {
	return { 
		q1.c * q2.c - q1.i * q2.i - q1.j * q2.j - q1.k * q2.k,
		q1.c * q2.i + q1.i * q2.c + q1.j * q2.k - q1.k * q2.j,
		q1.c * q2.j + q1.j * q2.c + q1.k * q2.i - q1.i * q2.k,
		q1.c * q2.k + q1.k * q2.c + q1.i * q2.j - q1.j * q2.i,
	};
}

quaternion operator*(double c, quaternion q) {
	return { c * q.c, c * q.i, c * q.j, c * q.k};
}

quaternion operator*(quaternion q, double c) {
	return { c * q.c, c * q.i, c * q.j, c * q.k };
}

quaternion operator+(quaternion q1, quaternion q2) {
	return {q1.c + q2.c, q1.i + q2.i, q1.j + q2.j, q1.k + q2.k };
}

quaternion operator-(quaternion q1, quaternion q2) {
	return { q1.c - q2.c, q1.i - q2.i, q1.j - q2.j, q1.k - q2.k };
}

quaternion operator-(quaternion q) {
	return { -q.c, -q.i, -q.j, -q.k };
}

v3 Rotate(v3 Vector, quaternion Q) {
	double S = Q.c;
	v3 U = V3(Q.i, Q.j, Q.k);
	return 2.0 * ((U * Vector) * U + S * cross(U, Vector)) + (S*S - U * U) * Vector;
}

basis Rotate(basis Basis, quaternion Q) {
	return {
		Rotate(Basis.X, Q),
		Rotate(Basis.Y, Q),
		Rotate(Basis.Z, Q)
	};
}