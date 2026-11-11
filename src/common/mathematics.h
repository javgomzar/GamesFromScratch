#pragma once

#include <platform.h>
#include <tokenizer.h>
#include "enum.h"


// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Constants                                                                                                                              |
// +----------------------------------------------------------------------------------------------------------------------------------------+

inline const float PI = 3.1415926535897932f;
inline const float TAU = 6.2831853071795865f;

// Twelfth root of 2, important for music
inline const float TWELFTH_ROOT_TWO = 1.0594630943592952646f;
inline const float SQRT_TWO = 1.41421356237f;
inline const float SQRT_TWO_INV = .7071067811865475244f;

inline const float DEGREES = PI / 180.0f;

// Smallest meaningful difference between floats, important for avoiding floating point rounding errors.
inline const float EPSILON = .00001f;

int32 Round(float x);
int32 Sign(float x);
float Clamp(float value, float min, float max);
int32 Clamp(int32 value, int32 min, int32 max);
uint32 Clamp(uint32 value, uint32 min, uint32 max);
float NormalizeAngle(float angle);
int8 GetExponent32(float number);
uint32 GetMantissa32(float number);
int16 GetExponent64(double number);
uint64 GetMantissa64(double number);
uint32 log2(uint32 x);
uint32 log2(uint64 x);
float BuildFloat(bool negative, int8 exponent, uint32 mantissa);
double BuildFloat(bool Negative, int16 Exponent, uint64 Mantissa);

class RNG {
private:
	static uint64 seed;
	static uint64 state;
    static void NextState();
    static int _RandomEnum(Type type);
public:
    static uint64 Seed();
    static float RandFloat();
    static float RandFloat(float min, float max);
    static bool Bernoulli(float p);
    static int RandInt(int min, int max);
    static float Normal();
    static float Normal(float Mean, float StdDeviation);
};

#define RandomEnum(type) (Type)_RandomEnum(Type_##type)

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| Vector                                                                                                                                 |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

struct V2 {
	float x, y;
    V2(float X, float Y);
    V2 operator-();
    V2& operator+=(V2 rhs);
    V2& operator-=(V2 rhs);
    V2& operator*=(float rhs);
    V2& operator/=(float rhs);
};

V2 operator+(V2 lhs, V2 rhs);
V2 operator-(V2 lhs, V2 rhs);
V2 operator*(float lhs, V2 rhs);
V2 operator*(V2 lhs, V2 rhs);
bool operator==(V2 lhs, V2 rhs);
bool operator!=(V2 lhs, V2 rhs);

V2 Radial(float angle);
float Dot(V2 lhs, V2 rhs);
float Cross(V2 lhs, V2 rhs);
bool AreParallel(V2 lhs, V2 rhs);
float Modulus(V2 v);
float GetAngle(V2 lhs, V2 rhs);
V2 Normalize(V2 v);
V2 Project(V2 lhs, V2 rhs);
float Distance(V2 lhs, V2 rhs);
V2 Perpendicular(V2 v);

struct UV2 {
	uint32 x, y;
    UV2(uint32 X, uint32 Y);
};

UV2 operator+(UV2 lhs, UV2 rhs);
bool operator==(UV2 lhs, UV2 rhs);

struct IV2 {
	int x, y;
    IV2(int X, int Y);
};

IV2 operator+(IV2 lhs, IV2 rhs);
bool operator==(IV2 lhs, IV2 rhs);
bool operator<(IV2 lhs, IV2 rhs);

struct V3 {
	float x, y, z;
    V3(float X, float Y, float Z);
    V3(float X, V2 V);
    V3(V2 V, float Z);
    V3 operator-();
    V3& operator+=(V3 rhs);
    V3& operator-=(V3 rhs);
    V3& operator*=(float rhs);
    V3& operator/=(float rhs);
};

V3 operator+(V3 lhs, V3 rhs);
V3 operator-(V3 lhs, V3 rhs);
V3 operator*(float lhs, V3 rhs);
V3 operator*(V3 lhs, V3 rhs);
V3 operator/(V3 lhs, float rhs);
bool operator==(V3 lhs, V3 rhs);
bool operator!=(V3 lhs, V3 rhs);
V3 Radial(float angle, float pitch);
float Dot(V3 lhs, V3 rhs);
V3 Cross(V3 A, V3 B);
float Modulus(V3 v);
float Distance(V3 lhs, V3 rhs);
V3 Normalize(V3 v);
V3 Project(V3 lhs, V3 rhs);
V3 DirectionFromAngle(float angle);
V3 SphereNormal(float theta, float phi);
V3 Rotate(V3 v, float angle);
V3 Rotate(V3 v, V3 w, float angle);

struct UV3 {
	uint32 x, y, z;
    UV3(uint32 X, uint32 Y, uint32 Z);
};

UV3 operator+(UV3 lhs, UV3 rhs);
bool operator==(UV3 lhs, UV3 rhs);

struct IV3 {
	int x, y, z;
    IV3(int X, int Y, int Z);
};

bool operator==(IV3 lhs, IV3 rhs);
bool operator<(IV3 lhs, IV3 rhs);

struct V4 {
	float x, y, z, w;
    V4(float X, float Y, float Z, float W);
    V4(V3 V, float W);
    V4(V2 U, V2 V);
    V4(float X, V3 V);
    V4 operator-();
    V4& operator+=(V4 rhs);
    V4& operator-=(V4 rhs);
    V4& operator*=(float rhs);
    V4& operator/=(float rhs);
};

V4 operator+(V4 lhs, V4 rhs);
V4 operator-(V4 lhs, V4 rhs);
V4 operator*(V4 lhs, V4 rhs);
V4 operator*(float lhs, V4 rhs);
V4 operator/(V4 lhs, float rhs);
float Dot(V4 lhs, V4 rhs);

struct IV4 {
	int x, y, z, w;
    IV4(int X, int Y, int Z, int W);
};

IV4 operator+(IV4 lhs, IV4 rhs);
bool operator==(IV4 lhs, IV4 rhs);
bool operator<(IV4 lhs, IV4 rhs);

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| Matrices                                                                                                                               |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

union Matrix2 {
	struct {
		float xx,xy,
		      yx,yy;
	};
	float array[4];
	struct {
		V2 row[2];
	};
	struct {
		V2 x, y;
	};
	struct {
		float element[2][2];
	};
    Matrix2(float XX, float XY, float YX, float YY);
    Matrix2(V2 X, V2 Y);
    static const Matrix2 ID;
    Matrix2 operator-();
    Matrix2& operator*=(float f);
    V2 Col(int i);
    Matrix2 Transpose();
    float Det();
    Matrix2 Inverse();
};

Matrix2 operator*(float lhs, Matrix2 rhs);
Matrix2 operator/(Matrix2 lhs, float rhs);
V2 operator*(Matrix2 lhs, V2 rhs);
V2 operator*(V2 lhs, Matrix2 rhs);
Matrix2 operator*(Matrix2 lhs, Matrix2 rhs);
bool operator==(Matrix2 lhs, Matrix2 rhs);
float Det(Matrix2 A);

union Matrix3 {
	struct {
		float xx,xy,xz,
		      yx,yy,yz,
			  zx,zy,zz;
	};
	float array[9];
	struct {
		V3 row[3];
	};
	struct {
		V3 x, y, z;
	};
	struct {
		float element[3][3];
	};
    Matrix3(
        float XX, float XY, float XZ,
        float YX, float YY, float YZ,
        float ZX, float ZY, float ZZ
    );
    Matrix3(V3 X, V3 Y, V3 Z);
    static const Matrix3 ID;
    Matrix3 operator-();
    Matrix3& operator*=(float f);
    V3 Col(int i);
    Matrix3 Transpose();
    float Det();
    Matrix3 Adjugate();
    Matrix3 Inverse();
};

Matrix3 operator*(float lhs, Matrix3 rhs);
Matrix3 operator/(Matrix3 lhs, float rhs);
V3 operator*(Matrix3 lhs, V3 rhs);
V3 operator*(V3 lhs, Matrix3 rhs); 
Matrix3 operator*(Matrix3 lhs, Matrix3 rhs);
bool operator==(Matrix3 lhs, Matrix3 rhs);
Matrix3 Rotation(float angle, V3 axis);

typedef Matrix3 Basis;

V3 ChangeBasis(V3 v, Basis basis);
Basis Rotate(Basis basis, float angle);
Basis Rotate(Basis basis, V3 w);
Basis Complete(V3 x, V3 y);
Basis Complete(V3 x);
Basis Normalize(Basis basis);

union Matrix4 {
	struct {
		float xx,xy,xz,xw,
		      yx,yy,yz,yw,
			  zx,zy,zz,zw,
			  wx,wy,wz,ww;
	};
	float array[16];
	struct {
		V4 row[4];
	};
	struct {
		V4 x, y, z, w;
	};
	struct {
		float element[4][4];
	};
    Matrix4(
	    float XX, float XY, float XZ, float XW,
	    float YX, float YY, float YZ, float YW,
	    float ZX, float ZY, float ZZ, float ZW,
	    float WX, float WY, float WZ, float WW
    );
    Matrix4(V4 X, V4 Y, V4 Z, V4 W);
    Matrix4(Matrix3 m);
    static const Matrix4 ID;
    V4 Col(int i);
    Matrix3 Submatrix();
    Matrix4 Transpose();
};

Matrix4 operator*(Matrix4 lhs, Matrix4 rhs);
V4 operator*(Matrix4 lhs, V4 rhs);
V4 operator*(V4 lhs, Matrix4 rhs);
bool operator==(Matrix4 lhs, Matrix4 rhs);

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| Complex numbers                                                                                                                        |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

struct Complex {
	float r; // Real part
	float i; // Imaginary part
    Complex(float R, float I);
    Complex& operator+=(Complex rhs);
    Complex& operator-=(Complex rhs);
    Complex& operator*=(Complex rhs);
    Complex& operator/=(Complex rhs);
    Complex Conjugate();
    Complex Inverse();
    float Modulus();
    float Phase();
};

Complex operator+(Complex lhs, Complex rhs);
Complex operator-(Complex lhs, Complex rhs);
Complex operator*(Complex lhs, Complex rhs);
Complex operator/(Complex lhs, Complex rhs);

Complex Expi(float alpha);

struct Quaternion {
	float c;
	float i;
	float j;
	float k;
    Quaternion(float C, float I, float J, float K);
    Quaternion(float angle, V3 vector);
    Quaternion Conjugate();
};

inline const Quaternion I = Quaternion(0.0, 1.0, 0.0, 0.0);
inline const Quaternion J = Quaternion(0.0, 0.0, 1.0, 0.0);
inline const Quaternion K = Quaternion(0.0, 0.0, 0.0, 1.0);

Quaternion operator*(Quaternion lhs, Quaternion rhs);
