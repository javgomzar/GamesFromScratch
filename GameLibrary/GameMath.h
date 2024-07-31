#pragma once
#include "math.h"

// Utility macros
#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) ((uint64)Megabytes(Value)*1024)

// Platform independent constants
static float Pi = 3.14159265359f;
static float Tau = 2.0f * Pi;
static float twroot = 1.05946309436f;

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

struct basis {
	v3 X;
	v3 Y;
	v3 Z;
};

basis Rotate(basis Basis, double Angle) {
	basis Result;
	Result.X = Rotate(Basis.X, Angle);
	Result.Y = Rotate(Basis.Y, Angle);
	Result.Z = Rotate(Basis.Z, Angle);
	return Result;
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