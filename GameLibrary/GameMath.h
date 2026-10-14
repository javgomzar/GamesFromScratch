#ifndef GAME_MATH
#define GAME_MATH

#pragma once

#include "GamePlatform.h"
#include "GameEnums.h"
#include "Tokenizer.h"


// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Constants                                                                                                                              |
// +----------------------------------------------------------------------------------------------------------------------------------------+

const float PI = 3.1415926535897932f;
const float TAU = 6.2831853071795865f;

// Twelfth root of 2, important for music
const float TWELFTH_ROOT_TWO = 1.0594630943592952646f;
const float SQRT_TWO = 1.41421356237f;
const float SQRT_TWO_INV = .7071067811865475244f;

const float DEGREES = PI / 180.0f;

// Smallest meaningful difference between floats, important for avoiding floating point rounding errors.
const float EPSILON = .00001f;

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Arithmetic                                                                                                                             |
// +----------------------------------------------------------------------------------------------------------------------------------------+

inline int32 CustomRound(float x) {
	return x >= 0.0f ? (int32)(x + 0.5f) : (int32)(x - 0.5f);
}

inline int32 Sign(float x) {
	if      (x > 0) return 1;
	else if (x < 0) return -1;
	return 0;
}

inline float Clamp(float value, float min, float max) {
	if      (value < min) return min;
	else if (value > max) return max;
	return value;
}

inline int32 Clamp(int32 value, int32 min, int32 max) {
	if      (value < min) return min;
	else if (value > max) return max;
	return value;
}

inline uint32 Clamp(uint32 value, uint32 min, uint32 max) {
	if      (value < min) return min;
	else if (value > max) return max;
	return value;
}

// Makes an angle be in the [0, 360) interval.
inline float Normalizeangle(float angle) {
	if (angle >= 360) {
		return angle - 360;
	}
	else if (angle < 0) {
		return angle + 360;
	}
	return angle;
}

inline int8 GetExponent32(float number) {
	uint32 Bytes;
	memcpy(&Bytes, &number, 4);
	return ((Bytes & 0x7F800000) >> 23) - 127;
}

inline uint32 GetMantissa32(float number) {
	uint32 Bytes;
	memcpy(&Bytes, &number, 4);
	return Bytes & 0x7FFFFF;
}

inline int16 GetExponent64(double number) {
	uint64 Bytes;
	memcpy(&Bytes, &number, 8);
	return ((Bytes & 0x7FF0000000000000ULL) >> 52) - 1023;
}

inline uint64 GetMantissa64(double number) {
	uint64 Bytes;
	memcpy(&Bytes, &number, 8);
	return Bytes & 0xFFFFFFFFFFFFFULL;
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | RNG                                                                                                                                    |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct RNG {
	uint64 seed;
	uint64 state;
};

RNG rng = {};

void RNGNextState() {
	rng.state ^= rng.state << 13;
	rng.state ^= rng.state >> 7;
	rng.state ^= rng.state << 17;
}

inline float RandFloat() {
	float result = (float)rng.state / (float)UINT64_MAX;
	RNGNextState();
	return result;
}

inline float RandFloat(float min, float max) {
	Assert(min <= max);
	return RandFloat() * (max - min) + min;
}

inline bool Bernoulli(float p = 0.5f) {
	return RandFloat() >= p;
}

// Returns a random integer greater or equal than min and strictly under max.
inline int RandInt(int min, int max) {
	return RandFloat() * (max - min) + min;
}

int _RandomEnum(Type type) {
    int i = ENUM_VALUES_SIZE;
    while(i > 0) {
        Debug_Enum_Value entry = ENUM_VALUES[--i];
        if (entry.enum_type == type) {
            return RandInt(0, entry.value);
        }
    }
    Assert(false, "Type wasn't found in enum list.");
    return 0;
}

#define RandomEnum(Type) (Type)_RandomEnum(debug_##Type)

float Normal() {
	static bool UseSaved = false;
	static float Savedvalue = 0.0f;

	if (UseSaved) {
		UseSaved = false;
		return Savedvalue;
	}

	float R = sqrtf(-2 * logf(RandFloat()));
	float Theta = TAU * RandFloat();

	UseSaved = true;
	Savedvalue = R * sinf(Theta);
	return R * cosf(Theta);
}

float Normal(float Mean, float StdDeviation) {
	return Mean + StdDeviation * Normal();
}

uint32 log2(uint32 x) {
	unsigned long result = 0;
	if (x > 0) {
#if _WIN32
	_BitScanReverse(&result, x);
#else
		result = __builtin_clzl(X);
#endif
	}
	return result;
}

uint32 log2(uint64 x) {
	unsigned long result = 0;
	if (x > 0) {
#if _WIN32
		_BitScanReverse64(&result, x);
#else
		result = __builtin_clzll(X);
#endif
	}
	return result;
}

/*
	IEEE-754 32 bits floating-point standard from sign, exponent and mantissa.
*/
float BuildFloat(bool Negative, int8 Exponent, uint32 Mantissa) {
	uint32 Exponent32 = 0;
	if (Exponent != -127 || Mantissa != 0) {
		Exponent32 = Exponent + 127;
	}
	uint32 value = ((uint32)Negative << 31) | (Exponent32 << 23) | (Mantissa & 0x7FFFFF);
	float result;
	memcpy(&result, &value, 4);
	return result;
}

/*
	IEEE-754 64 bits floating-point standard from sign, exponent and mantissa.
*/
double BuildFloat(bool Negative, int16 Exponent, uint64 Mantissa) {
	uint64 Exponent64 = 0;
	if (Exponent != -1023 || Mantissa != 0) {
		Exponent64 = Exponent + 1023;
	}
	uint64 value = ((uint64)Negative << 63) | (Exponent64 << 52) | (Mantissa & 0xFFFFFFFFFFFFFULL);
	double result;
	memcpy(&result, &value, 8);
	return result;
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | 2D                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct UV2 {
	uint32 x, y;
};

inline UV2 operator+(UV2 a, UV2 b) {
	return {a.x + b.x, a.y + b.y};
}

inline bool operator==(const UV2& lhs, const UV2& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

struct IV2 {
	int x, y;
};

inline IV2 operator+(IV2 a, IV2 b) {
	return IV2(a.x + b.x, a.y + b.y);
}

inline bool operator==(const IV2& lhs, const IV2& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator<(const IV2& lhs, const IV2& rhs) {
	bool result = 0;
	if (lhs.x == rhs.x) {
		if (lhs.y == rhs.y) {
			return false;
		}
		else return lhs.y < rhs.y;
	}
	else return lhs.x < rhs.x;
}

struct V2 {
	float x, y;
};

inline V2 Radial(float angle) {
	return {cosf(angle), sinf(angle)};
}

inline V2 operator+(V2 A, V2 B) {
	V2 result;
	result.x = A.x + B.x;
	result.y = A.y + B.y;
	return result;
}

inline V2 operator-(V2 A) {
	V2 result;
	result.x = -A.x;
	result.y = -A.y;
	return result;
}

inline V2 operator-(V2 A, V2 B) {
	V2 result;
	result.x = A.x - B.x;
	result.y = A.y - B.y;
	return result;
}

inline V2 operator*(float C, V2 A) {
	V2 result;
	result.x = C * A.x;
	result.y = C * A.y;
	return result;
}

inline V2 operator*(V2 A, V2 B) {
	return { A.x * B.x , A.y * B.y };
}

inline bool operator==(V2 A, V2 B) {
	return fabsf(A.x - B.x) < EPSILON && fabsf(A.y - B.y) < EPSILON;
}

inline bool operator!=(V2 A, V2 B) {
	return fabsf(A.x - B.x) >= EPSILON || fabsf(A.y - B.y) >= EPSILON;
}

inline V2& operator+=(V2& A, V2 B) {
	A.x += B.x;
	A.y += B.y;
	return A;
}

inline V2& operator/=(V2& A, float C) {
	A.x /= C;
	A.y /= C;
	return A;
}

inline V2& operator-=(V2& A, V2 B) {
	A.x -= B.x;
	A.y -= B.y;
	return A;
}

inline V2& operator*=(V2& A, float C) {
	A.x *= C;
	A.y *= C;
	return A;
}

inline float dot(V2 A, V2 B) {
	return A.x * B.x + A.y * B.y;
}

inline float cross(V2 A, V2 B) {
	return A.x * B.y - A.y * B.x;
}

inline bool AreParallel(V2 A, V2 B) {
	float C = cross(A, B);
	return fabsf(C) < EPSILON;
}

inline float modulus(V2 A) {
	return sqrt(dot(A, A));
}

inline float Getangle(V2 A, V2 B) {
	float mA = modulus(A);
	float mB = modulus(B);
	if (mA < EPSILON || mB < EPSILON) {
		return 0;
	}
	float s = cross(A, B) / mA / mB;
	float c = dot(A, B) / mA / mB;
	return atan2f(s, c);
}

inline V2 normalize(V2 V) {
	return (modulus(V) < EPSILON) ? V2(0,0) : (1 / modulus(V)) * V;
}

inline V2 project(V2 A, V2 B) {
	V2 N = normalize(B);
	return dot(A, N) * N;
}

inline float distance(V2 A, V2 B) {
	return modulus(A - B);
}
 
inline V2 perp(V2 A) {
	return { -A.y, A.x };
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | 3D                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct UV3 {
	uint32 x, y, z;
};

inline UV3 operator+(UV3 a, UV3 b) {
	return UV3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline bool operator==(const UV3& lhs, const UV3& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

struct IV3 {
	int x, y, z;
};

inline IV3 operator+(IV3 a, IV3 b) {
	return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline bool operator==(const IV3& lhs, const IV3& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator<(const IV3& lhs, const IV3& rhs) {
	bool result = 0;
	if (lhs.x == rhs.x) {
		if (lhs.y == rhs.y) {
			if (lhs.z == rhs.z) return false;
			else return lhs.z < rhs.z;
		}
		else return lhs.y < rhs.y;
	}
	else return lhs.x < rhs.x;
}

struct V3 {
	float x, y, z;
};

inline V3 Radial(float angle, float Pitch) {
	float cosA = cosf(angle * DEGREES);
    float sinA = sinf(angle * DEGREES);
    float cosP = cosf(Pitch * DEGREES);
    float sinP = sinf(Pitch * DEGREES);
    return {sinA * cosP, sinP, cosA * cosP};
}

inline V3 operator+(V3 A, V3 B) {
	return {A.x + B.x, A.y + B.y, A.z + B.z};
}

inline V3 operator-(V3 A) {
	V3 result;
	result.x = -A.x;
	result.y = -A.y;
	result.z = -A.z;
	return result;
}

inline V3 operator-(V3 A, V3 B) {
	V3 result;
	result.x = A.x - B.x;
	result.y = A.y - B.y;
	result.z = A.z - B.z;
	return result;
}

inline V3 operator*(float C, V3 A) {
	V3 result;
	result.x = C * A.x;
	result.y = C * A.y;
	result.z = C * A.z;
	return result;
}

 inline V3 operator*(V3 A, float C) {
	V3 result;
	result.x = C * A.x;
	result.y = C * A.y;
	result.z = C * A.z;
	return result;
}

 inline V3 operator*(V3 A, V3 B) {
	 return V3(A.x * B.x, A.y * B.y, A.z * B.z);
 }

inline V3 operator/(V3 A, float C) {
	V3 result;
	result.x = A.x / C;
	result.y = A.y / C;
	result.z = A.z / C;
	return result;
}

inline V3& operator+=(V3& A, V3 B) {
	A.x += B.x;
	A.y += B.y;
	A.z += B.z;
	return A;
}

inline V3& operator/=(V3& A, float C) {
	A.x /= C;
	A.y /= C;
	A.z /= C;
	return A;
}

inline V3& operator-=(V3& A, V3 B) {
	A.x -= B.x;
	A.y -= B.y;
	A.z -= B.z;
	return A;
}

inline V3& operator*=(V3& A, float C) {
	A.x *= C;
	A.y *= C;
	A.z *= C;
	return A;
}

inline bool operator==(V3 A, V3 B) {
	return fabsf(A.x - B.x) < EPSILON && fabsf(A.y - B.y) < EPSILON && fabsf(A.z - B.z) < EPSILON;
}

inline bool operator!=(V3 A, V3 B) {
	return fabsf(A.x - B.x) >= EPSILON || fabsf(A.y - B.y) >= EPSILON || fabsf(A.z - B.z) >= EPSILON;
}

inline float dot(V3 A, V3 B) {
	return A.x * B.x + A.y * B.y + A.z * B.z;
}

inline V3 cross(V3 A, V3 B) {
	V3 result;
	result.x = A.y * B.z - A.z * B.y;
	result.y = A.z * B.x - A.x * B.z;
	result.z = A.x * B.y - A.y * B.x;
	return result;
}

inline float modulus(V3 A) {
	return sqrt(dot(A, A));
}

inline float distance(V3 A, V3 B) {
	return modulus(B - A);
}

inline V3 normalize(V3 V) {
	return (modulus(V) < EPSILON) ? V3(0,0,0) : (1.0f / modulus(V)) * V;
}

inline V3 project(V3 A, V3 B) {
	return dot(A, B) * B / dot(B, B);
}

inline V3 DirectionFromangle(float Theta) {
	V3 result;
	result.x = cosf(Theta);
	result.y = sinf(Theta);
	result.z = 0;
	return result;
}

inline V3 SphereNormal(float Theta, float Phi) {
	V3 result;
	result.x = cosf(Phi)*sinf(Theta);
	result.y = sinf(Phi)*sinf(Theta);
	result.z = cosf(Theta);
	return result;
}

inline V3 Rotate(V3 v, float angle) {
	float x = v.x * cosf(angle) + v.y * sinf(angle);
	float y = -v.x * sinf(angle) + v.y * cosf(angle);
	float z = v.z;
	return V3(x, y, z);
}

inline V3 Rotate(V3 v, V3 w, float angle = 0.0) {
	if (angle == 0.0) {
		float angle = TAU * modulus(w);
	}
	V3 n = normalize(w);
	// Rodrigues formula
	return cosf(angle) * v + sinf(angle) * cross(n, v) + (1 - cosf(angle)) * (n * v) * n;
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | 4D                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct IV4 {
	int x, y, z, w;
};

inline IV4 operator+(IV4 a, IV4 b) {
	return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

inline bool operator==(const IV4& lhs, const IV4& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator<(const IV4& lhs, const IV4& rhs) {
	bool result = 0;
	if (lhs.x == rhs.x) {
		if (lhs.y == rhs.y) {
			if (lhs.z == rhs.z) return false;
			else return lhs.z < rhs.z;
		}
		else return lhs.y < rhs.y;
	}
	else return lhs.x < rhs.x;
}

struct V4 {
	float x, y, z, w;
};

inline V4 operator+(V4 A, V4 B) {
	return {A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w};
}

inline V4 operator-(V4 A) {
	return {-A.x, -A.y, -A.z, -A.w};
}

inline V4 operator-(V4 A, V4 B) {
	return {A.x - B.x, A.y - B.y, A.z - B.z, A.w - B.w};
}

inline V4 operator*(float C, V4 A) {
	return {C * A.x, C * A.y, C * A.z, C * A.w};
}

 inline V4 operator*(V4 A, V4 B) {
	return {A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w};
 }

 inline V4 operator/(V4 A, float C) {
	return {A.x / C, A.y / C, A.z / C, A.w / C};
 }

inline V4& operator+=(V4& A, V4 B) {
	A.x += B.x;
	A.y += B.y;
	A.z += B.z;
	A.w += B.w;
	return A;
}

inline V4& operator-=(V4& A, V4 B) {
	A.x -= B.x;
	A.y -= B.y;
	A.z -= B.z;
	A.w -= B.w;
	return A;
}

inline V4& operator*=(V4& A, float C) {
	A.x *= C;
	A.y *= C;
	A.z *= C;
	A.w *= C;
	return A;
}

inline V4& operator/=(V4& A, float C) {
	A.x /= C;
	A.y /= C;
	A.z /= C;
	A.w /= C;
	return A;
}


inline float dot(V4 A, V4 B) {
	return A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Matrices                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------+

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
};

Matrix2 identity2 = {
	1,0,
	0,1
};

inline V2 col(Matrix2 m, int i) {
	switch(i) {
		case 0: return V2(m.xx, m.yx); break;
		case 1: return V2(m.xy, m.yy); break;
		default: Assert(false); return V2(0,0);
	};
}

inline Matrix2 transpose(Matrix2 m) {
	Matrix2 result;
	result.x = col(m, 0);
	result.y = col(m, 1);

	return result;
}

inline Matrix2 operator*(float c, Matrix2 A) {
	Matrix2 result = A;
	result.x *= c;
	result.y *= c;
	return result;
}

inline Matrix2 operator-(Matrix2 A) {
	Matrix2 result = A;
	result.x = -result.x;
	result.y = -result.y;
	return result;
}

inline Matrix2 operator*(Matrix2 A, Matrix2 B) {
	Matrix2 result;
	result.xx = A.xx * B.xx + A.xy * B.yx;
	result.xy = A.xx * B.xy + A.xy * B.yy;
	result.yx = A.yx * B.xx + A.yy * B.yx;
	result.yy = A.yx * B.xy + A.yy * B.yy;
	return result;
}

inline V2 operator*(Matrix2 A, V2 V) {
	V2 result;
	result.x = dot(A.x, V);
	result.y = dot(A.y, V);
	return result;
}

inline V2 operator*(V2 V, Matrix2 A) {
	V2 result;
	result.x = dot(V, col(A, 0));
	result.y = dot(V, col(A, 1));
	return result;
}

inline Matrix2 operator/(Matrix2 A, float c) {
	Matrix2 result = A;
	result.x /= c;
	result.y /= c;
	return result;
}

inline Matrix2& operator*=(Matrix2& A, float c) {
	A.x *= c;
	A.y *= c;
	return A;
}

inline bool operator==(Matrix2 A, Matrix2 B) {
	for (int i = 0; i < 4; i++) {
		if (fabsf(A.array[i] - B.array[i]) > EPSILON) return false;
	}
	return true;
}

inline float det(Matrix2 A) {
	return A.xx * A.yy - A.xy * A.yx;
}

inline Matrix2 inverse(Matrix2 A) {
	float D = det(A);
	Assert(fabsf(D) > EPSILON);
	Matrix2 result = {
		A.yy / D, -A.xy / D,
		-A.yx / D, A.xx / D
	};
	return result;
}

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
};

Matrix3 Identity3 = {
	1,0,0,
	0,1,0,
	0,0,1
};

inline V3 col(Matrix3 A, int i) {
	switch(i) {
		case 0: return V3(A.xx, A.yx, A.zx); break;
		case 1: return V3(A.xy, A.yy, A.zy); break;
		case 2: return V3(A.xz, A.yz, A.zz); break;
		default: Assert(false); return V3(0,0,0);
	};
}

inline Matrix3 transpose(Matrix3 A) {
	Matrix3 result;
	result.x = col(A, 0);
	result.y = col(A, 1);
	result.z = col(A, 2);

	return result;
}

inline Matrix3 operator*(float c, Matrix3 A) {
	Matrix3 result = A;
	result.x *= c;
	result.y *= c;
	result.z *= c;
	return result;
}

inline Matrix3 operator-(Matrix3 A) {
	Matrix3 result = A;
	result.x = -result.x;
	result.y = -result.y;
	result.z = -result.z;
	return result;
}

inline Matrix3 operator*(Matrix3 A, Matrix3 B) {
	Matrix3 result;
	result.xx = dot(A.x, col(B, 0));
	result.xy = dot(A.x, col(B, 1));
	result.xz = dot(A.x, col(B, 2));
	result.yx = dot(A.y, col(B, 0));
	result.yy = dot(A.y, col(B, 1));
	result.yz = dot(A.y, col(B, 2));
	result.zx = dot(A.z, col(B, 0));
	result.zy = dot(A.z, col(B, 1));
	result.zz = dot(A.z, col(B, 2));
	return result;
}

inline V3 operator*(Matrix3 A, V3 V) {
	V3 result;
	result.x = dot(A.x, V);
	result.y = dot(A.y, V);
	result.z = dot(A.z, V);
	return result;
}

inline V3 operator*(V3 V, Matrix3 A) {
	V3 result;
	result.x = dot(V, col(A, 0));
	result.y = dot(V, col(A, 1));
	result.z = dot(V, col(A, 2));
	return result;
}

inline Matrix3 operator/(Matrix3 A, float c) {
	Matrix3 result = A;
	result.x /= c;
	result.y /= c;
	result.z /= c;
	return result;
}

inline Matrix3& operator*=(Matrix3& A, float c) {
	A.x *= c;
	A.y *= c;
	A.z *= c;
	return A;
}

inline bool operator==(Matrix3 A, Matrix3 B) {
	for (int i = 0; i < 9; i++) {
		if (fabsf(A.array[i] - B.array[i]) >= EPSILON) return false;
	}
	return true;
}

inline float det(Matrix3 A) {
	return A.xx * A.yy * A.zz + A.xy * A.yz * A.zx + A.xz * A.yx * A.zy - A.xz * A.yy * A.zx  - A.xy * A.yx * A.zz - A.xx * A.yz * A.zy;
}

inline Matrix3 adjugate(Matrix3 A) {
	Matrix3 result;
	result.xx = A.yy * A.zz - A.yz * A.zy;
	result.xy = A.yz * A.zx - A.yx * A.zz;
	result.xz = A.yx * A.zy - A.yy * A.zx;
	result.yx = A.xz * A.zy - A.xy * A.zz;
	result.yy = A.xx * A.zz - A.xz * A.zx;
	result.yz = A.xy * A.zx - A.xx * A.zy;
	result.zx = A.xy * A.yz - A.xz * A.yy;
	result.zy = A.xz * A.yx - A.xx * A.yz;
	result.zz = A.xx * A.yy - A.xy * A.yx;
	return result;
}

inline Matrix3 inverse(Matrix3 A) {
	Assert(fabsf(det(A)) > EPSILON);
	return adjugate(A) / det(A);
}

inline Matrix3 Rotation(float angle, V3 Axis) {
	V3 u = normalize(Axis);

	float cos_ = cosf(angle);
	float sin_ = sinf(angle);

	Matrix3 result;
	result.xx = cos_ + u.x * u.x * (1 - cos_);
	result.xy = u.x * u.y * (1 - cos_) - u.z * sin_;
	result.xz = u.x * u.z * (1 - cos_) + u.y * sin_;
	result.yx = u.y * u.x * (1 - cos_) + u.z * sin_;
	result.yy = cos_ + u.y * u.y * (1 - cos_);
	result.yz = u.y * u.z * (1 - cos_) - u.x * sin_;
	result.zx = u.z * u.x * (1 - cos_) - u.y * sin_;
	result.zy = u.z * u.y * (1 - cos_) + u.x * sin_;
	result.zz = cos_ + u.z * u.z * (1 - cos_);
	return result;
}

typedef Matrix3 Basis;

inline V3 ChangeBasis(V3 V, Basis basis) {
	return V.x * basis.x + V.y * basis.y + V.z * basis.z;
}

inline Basis Rotate(Basis basis, float angle) {
	Basis result;
	result.x = Rotate(basis.x, angle);
	result.y = Rotate(basis.y, angle);
	result.z = Rotate(basis.z, angle);
	return result;
}

inline Basis Rotate(Basis basis, V3 w) {
	Basis result;
	result.x = Rotate(basis.x, w);
	result.y = Rotate(basis.y, w);
	result.z = Rotate(basis.z, w);
	return result;
}

inline Basis Complete(V3 x, V3 y) {
	Basis result;
	result.x = x;
	result.y = y;
	result.z = normalize(cross(x,y));
	return result;
}

inline Basis Complete(V3 x) {
	V3 y = cross(x, V3(1,0,0));
	if (modulus(y) < EPSILON) {
		y = cross(x, V3(0,1,0));
		if (modulus(y) < EPSILON) {
			y = cross(x, V3(0,0,1));
		}
	}
	return Complete(x, normalize(y));
}

inline Basis Identity(float Factor = 1.0) {
	Basis result;
	result.x = V3(Factor, 0.0, 0.0);
	result.y = V3(0.0, Factor, 0.0);
	result.z = V3(0.0, 0.0, Factor);
	return result;
}

inline Basis normalize(Basis basis) {
	Basis result;
	result.x = normalize(basis.x);
	result.y = normalize(basis.y);
	result.z = normalize(basis.z);
	return result;
}

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
};

Matrix4 identity4 = {
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};

inline V4 col(Matrix4 A, int i) {
	switch(i) {
		case 0: return V4(A.xx, A.yx, A.zx, A.wx); break;
		case 1: return V4(A.xy, A.yy, A.zy, A.wy); break;
		case 2: return V4(A.xz, A.yz, A.zz, A.wz); break;
		case 3: return V4(A.xw, A.yw, A.zw, A.ww); break;
		default: Assert(false); return V4(0,0,0,0);
	};
}

inline Matrix4 transpose(Matrix4 A) {
	Matrix4 result;
	result.x = col(A, 0);
	result.y = col(A, 1);
	result.z = col(A, 2);
	result.w = col(A, 3);
	return result;
}

inline Matrix4 operator*(Matrix4 A, Matrix4 B) {
	Matrix4 result;
	result.xx = dot(A.x, col(B, 0));
	result.xy = dot(A.x, col(B, 1));
	result.xz = dot(A.x, col(B, 2));
	result.xw = dot(A.x, col(B, 3));
	result.yx = dot(A.y, col(B, 0));
	result.yy = dot(A.y, col(B, 1));
	result.yz = dot(A.y, col(B, 2));
	result.yw = dot(A.y, col(B, 3));
	result.zx = dot(A.z, col(B, 0));
	result.zy = dot(A.z, col(B, 1));
	result.zz = dot(A.z, col(B, 2));
	result.zw = dot(A.z, col(B, 3));
	result.wx = dot(A.w, col(B, 0));
	result.wy = dot(A.w, col(B, 1));
	result.wz = dot(A.w, col(B, 2));
	result.ww = dot(A.w, col(B, 3));
	return result;
}

inline V4 operator*(Matrix4 A, V4 V) {
	V4 result;
	result.x = dot(A.x, V);
	result.y = dot(A.y, V);
	result.z = dot(A.z, V);
	result.w = dot(A.w, V);
	return result;
}

inline V4 operator*(V4 V, Matrix4 A) {
	V4 result;
	result.x = dot(col(A, 0), V);
	result.y = dot(col(A, 1), V);
	result.z = dot(col(A, 2), V);
	result.w = dot(col(A, 3), V);
	return result;
}

inline bool operator==(Matrix4 A, Matrix4 B) {
	for (int i = 0; i < 16; i++) {
		if (fabs(A.element[i] - B.element[i]) > EPSILON) return false;
	}
	return true;
}

inline Matrix3 Submatrix3(Matrix4 Matrix) {
	return {
		Matrix.xx, Matrix.xy, Matrix.xz,
		Matrix.yx, Matrix.yy, Matrix.yz,
		Matrix.zx, Matrix.zy, Matrix.zz
	};
}

inline Matrix4 Matrix4FromMatrix3(Matrix3 m) {
	Matrix4 result;
	result.x = V4(m.xx, m.xy, m.xz, 0);
	result.y = V4(m.yx, m.yy, m.yz, 0);
	result.z = V4(m.zx, m.zx, m.zz, 0);
	result.w = V4(0,0,0,1);
	return result;
}

Matrix4 GetScreenProjectionMatrix(float Width, float Height) {
	float a = 2.0f / Width;
	float b = 2.0f / Height;

	Matrix4 result = {
		   a, 0.0f, 0.0f, 0.0f,
		0.0f,   -b, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
	   -1.0f, 1.0f, 0.0f, 1.0f,
	};

	return result;
}

Matrix4 GetWorldProjectionMatrix(float Width, float Height) {
	float s_x = 1.0;
	float s_y = Width / Height;
	float s_z = 1.0;

	Matrix4 result = {
		  s_x, 0.0f, 0.0f, 0.0f,
		0.0f,   s_y, 0.0f, 0.0f,
		0.0f,  0.0f, 1.0f,   s_z,
		0.0f,  0.0f,-1.0f, 0.0f,
	};
	
	return result;
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Complex numbers                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct complex {
	float r; // Real part
	float i; // Imaginary part
};

complex Complex(float r, float i) {
	return {r, i};
}

inline complex conjugate(complex A) {
	return {
		A.r,
		-A.i
	};
}

inline complex operator+(complex A, float B) {
	return {
		A.r + B,
		A.i
	};
}

inline complex& operator+=(complex& A, float B) {
	A.r += B;
	return A;
}

inline complex operator+(float A, complex B) {
	return {
		A + B.r,
		B.i
	};
}

inline complex operator-(float A, complex B) {
	return {
		A - B.r,
		B.i
	};
}

inline complex operator-(complex A, float B) {
	return {
		A.r - B,
		A.i
	};
}

inline complex& operator-=(complex& A, float B) {
	A.r -= B;
	return A;
}

inline complex operator*(complex A, float C) {
	return {
		A.r * C,
		A.i * C
	};
}

inline complex operator*(float C, complex A) {
	return {
		A.r * C,
		A.i * C
	};
}

inline complex& operator*=(complex& A, float B) {
	A.r *= B;
	A.i *= B;
	return A;
}

inline complex operator/(complex A, float C) {
	return {
		A.r / C,
		A.i / C
	};
}

inline complex& operator/=(complex& A, float B) {
	A.r /= B;
	A.i /= B;
	return A;
}

inline float modulus(complex A) {
	return sqrt(A.r * A.r + A.i * A.i);
}

inline float phase(complex A) {
	return atan2f(A.i, A.r);
}

inline complex inverse(complex A) {
	return conjugate(A) / (A.r * A.r + A.i * A.i);
}

inline complex expi(float Alpha) {
	return {
		cos(Alpha),
		sin(Alpha)
	};
}

inline complex operator+(complex A, complex B) {
	return {
		A.r + B.r,
		A.i + B.i
	};
}

inline complex& operator+=(complex& A, complex B) {
	A.r += B.r;
	A.i += B.i;
	return A;
}

inline complex operator-(complex A, complex B) {
	return {
		A.r - B.r,
		A.i - B.i
	};
}

inline complex& operator-=(complex& A, complex B) {
	A.r -= B.r;
	A.i -= B.i;
	return A;
}

inline complex operator*(complex A, complex B) {
	return {
		A.r * B.r - A.i * B.i,
		A.r * B.i + A.i * B.r
	};
}

inline complex& operator*=(complex& A, complex B) {
	A = A * B;
	return A;
}

inline complex operator/(complex A, complex B) {
	return A * inverse(B);
}

inline complex operator/(float A, complex B) {
	return A * inverse(B);
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Quaternions                                                                                                                            |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct Quaternion {
	float c;
	float i;
	float j;
	float k;
};

const Quaternion I = { 0.0, 1.0, 0.0, 0.0 };
const Quaternion J = { 0.0, 0.0, 1.0, 0.0 };
const Quaternion K = { 0.0, 0.0, 0.0, 1.0 };

inline Quaternion GetQuaternion(float angle, V3 Vector) {
	float sin_angle = sinf(angle / 2);
	Vector = normalize(Vector);
	return { cosf(angle / 2.0), sin_angle * Vector.x, sin_angle * Vector.y, sin_angle * Vector.z};
}

inline Quaternion Conjugate(Quaternion q) {
	return { q.c, -q.i, -q.j, -q.k};
}

inline Quaternion operator*(Quaternion q1, Quaternion q2) {
	return { 
		q1.c * q2.c - q1.i * q2.i - q1.j * q2.j - q1.k * q2.k,
		q1.c * q2.i + q1.i * q2.c + q1.j * q2.k - q1.k * q2.j,
		q1.c * q2.j + q1.j * q2.c + q1.k * q2.i - q1.i * q2.k,
		q1.c * q2.k + q1.k * q2.c + q1.i * q2.j - q1.j * q2.i,
	};
}

inline Quaternion operator*(float c, Quaternion q) {
	return { c * q.c, c * q.i, c * q.j, c * q.k};
}

inline Quaternion operator*(Quaternion q, float c) {
	return { c * q.c, c * q.i, c * q.j, c * q.k };
}

inline Quaternion operator+(Quaternion q1, Quaternion q2) {
	return {q1.c + q2.c, q1.i + q2.i, q1.j + q2.j, q1.k + q2.k };
}

inline Quaternion operator-(Quaternion q1, Quaternion q2) {
	return { q1.c - q2.c, q1.i - q2.i, q1.j - q2.j, q1.k - q2.k };
}

inline Quaternion operator-(Quaternion q) {
	return { -q.c, -q.i, -q.j, -q.k };
}

inline V3 operator*(Quaternion Q, V3 Vector) {
	Quaternion Q_ = Conjugate(Q);
	Quaternion V = Quaternion(0.0, Vector.x, Vector.y, Vector.z);
	Quaternion result = Q_ * V * Q;
	return V3(result.i, result.j, result.k);
}

inline Basis operator*(Quaternion Q, Basis basis) {
	Basis result;
	result.x = Q * basis.x;
	result.y = Q * basis.y;
	result.z = Q * basis.z;

	return result;
}

inline bool operator==(Quaternion Q1, Quaternion Q2) {
	return Q1.c == Q2.c && Q1.i == Q2.i && Q1.j == Q2.j && Q1.k == Q2.k; 
}

inline bool operator!=(Quaternion Q1, Quaternion Q2) {
	return Q1.c != Q2.c || Q1.i == Q2.i || Q1.j == Q2.j || Q1.k == Q2.k; 
}

inline Matrix3 Matrix(Quaternion Q) {
	Matrix3 result;
	result.xx  = (2.0f * (Q.c * Q.c + Q.i * Q.i) - 1.0f);
	result.xy  = 2.0f * (Q.i * Q.j - Q.c * Q.k);
	result.xz  = 2.0f * (Q.i * Q.k + Q.c * Q.j);
	result.yx  = 2.0f * (Q.i * Q.j + Q.c * Q.k);
	result.yy  = (2.0f * (Q.c * Q.c + Q.j * Q.j) - 1.0f);
	result.yz  = 2.0f * (Q.j * Q.k - Q.c * Q.i);
	result.zx  = 2.0f * (Q.i * Q.k - Q.c * Q.j);
	result.zy  = 2.0f * (Q.j * Q.k + Q.c * Q.i);
	result.zz = (2.0f * (Q.c * Q.c + Q.k * Q.k) - 1.0f);
	return result;
}


// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Transform                                                                                                                              |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct Scale {
	float x;
	float y;
	float z;
};

inline Scale GetScale(float x = 1.0, float y = 1.0, float z = 1.0) {
	return { x, y, z };
};

inline Scale operator*(float C, Scale S) {
	return { C * S.x, C * S.y, C * S.z };
}

inline Scale operator*(Scale S, Scale T) {
	return GetScale(S.x * T.x, S.y * T.y, S.z * T.z);
}

inline V3 operator*(Scale S, V3 Vector) {
	return {S.x * Vector.x, S.y * Vector.y, S.z * Vector.z};
}

inline Basis operator*(Scale S, Basis basis) {
	Basis result;
	result.x = S.x * basis.x;
	result.y = S.y * basis.y;
	result.z = S.z * basis.z;
	return result;
}

inline bool operator==(Scale scale1, Scale scale2) {
	return scale1.x == scale2.x && scale1.y == scale2.y && scale1.z == scale2.z;
}

inline bool operator!=(Scale scale1, Scale scale2) {
	return scale1.x != scale2.x || scale1.y != scale2.y || scale1.z != scale2.z;
}

INTROSPECT
struct Transform {
	V3 translation;
	Scale scale;
	Quaternion rotation;
};

Transform IdentityTransform = { V3(0,0,0), GetScale(), Quaternion(1.0f) };

inline Transform GetTransform(Quaternion Rotation, V3 Translation = V3(0,0,0), Scale Scaling = GetScale()) {
	return { Translation, Scaling, Rotation };
}

inline Transform GetTransform(V3 Translation, Quaternion Rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f), Scale Scaling = GetScale()) {
	return { Translation, Scaling, Rotation };
}

inline Transform GetTransform(V3 Translation, Scale Scaling, Quaternion Rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f)) {
	return { Translation, Scaling, Rotation };
}

inline V3 operator*(Transform transform, V3 Vector) {
	return transform.rotation * (transform.scale * Vector) + transform.translation;
}

inline V4 operator*(Transform transform, V4 vector) {
	V3 v = transform.rotation * (transform.scale * V3(vector.x, vector.y, vector.z)) + vector.w * transform.translation;
	return { v.x, v.y, v.z, vector.w};
}

inline Basis operator*(Transform transform, Basis basis) {
	return transform.rotation * basis;
}

inline Transform operator*(Transform transform1, Transform transform2) {
	Transform result = { 0 };
	result.scale = transform1.scale * transform1.scale;
	result.translation = transform2.translation + transform2.rotation * transform2.translation;
	result.rotation = transform1.rotation * transform2.rotation;
	return result;
}

inline bool operator==(Transform transform1, Transform transform2) {
	return transform1.translation == transform2.translation && 
	       transform1.scale == transform2.scale && 
		   transform1.rotation == transform2.rotation;
}

inline bool operator!=(Transform transform1, Transform transform2) {
	return transform1.translation != transform2.translation || 
	       transform1.scale == transform2.scale || 
		   transform1.rotation == transform2.rotation;
}

inline Matrix4 Matrix(Transform transform) {
	Matrix3 rotation = Matrix(transform.rotation);
	Matrix4 result;
	result.x = transform.scale.x * V4(rotation.xx, rotation.xy, rotation.xz, 0);
	result.y = transform.scale.y * V4(rotation.yx, rotation.yy, rotation.yz, 0);
	result.z = transform.scale.z * V4(rotation.zx, rotation.zy, rotation.zz, 0);
	result.w = V4(transform.translation.x, transform.translation.y, transform.translation.z, 1.0f);
	return result;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Geometry                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct line2 {
	V2 Point;
	V2 Direction;
};

line2 Line(V2 Point, V2 Direction) {
	return { Point, normalize(Direction) };
}

line2 LineFromPoints(V2 A, V2 B) {
	return { A, normalize(B - A) };
}

bool AreParallel(line2 L1, line2 L2) {
	return AreParallel(L1.Direction, L2.Direction);
}

struct line3 {
	V3 Point;
	V3 Direction;
};

line3 Line(V3 Point, V3 Direction) {
	return { Point, normalize(Direction) };
}

line3 LineFromPoints(V3 A, V3 B) {
	return { A, normalize(B - A) };
}

inline V3 ClosestPoint(line3 Line, V3 Point) {
	float t = dot(Line.Direction, Point - Line.Point);
	return Point + t * Line.Direction;
}

inline float Distance(line3 Line, V3 Point) {
	return dot(Line.Direction, Point - Line.Point);
}

inline float Distance(line3 Line1, line3 Line2) {
	float a = dot(Line1.Direction, Line2.Direction);
	if (a == 0.0) { // Lines are parallel
		return Distance(Line1, Line2.Point);
	}
	else {
		V3 v = Line1.Point - Line2.Point;
		float b1 = dot(Line1.Direction, v);
		float b2 = dot(Line2.Direction, v);
		float s = (a * b2 - b1) / a;
		float t = (b2 - a * b1) / a;
		V3 Point1 = Line1.Point + s * Line1.Point;
		V3 Point2 = Line2.Point + t * Line2.Direction;
		return modulus(Point1 - Point2);
	}
}

struct Ray {
	V3 point;
	V3 direction;
};

Ray GetRay(V3 point, V3 direction) {
	return { point, normalize(direction) };
}

inline V3 ClosestPoint(Ray ray, V3 point) {
	float t = dot(ray.direction, point - ray.point);
	if (t <= 0.0f) return ray.point;
	else return ray.point + t * ray.direction;
}

inline float SqDistance(Ray ray, V3 point) {
	V3 D = point - ClosestPoint(ray, point);
	return dot(D,D);
}

struct segment2 {
	V2 Head;
	V2 Tail;
};

inline bool AreParallel(segment2 S1, segment2 S2) {
	return AreParallel(S1.Tail - S1.Head, S2.Tail - S2.Head);
}

inline bool IsInside(segment2 S, V2 P) {
	V2 u = S.Head - P;
	V2 v = S.Tail - P;

	return AreParallel(u,v) && dot(u,v) <= 0;
}

/*
	Intersects two 2D segments. If result is 0, there is no intersection. If result is 1, the intersection is exactly one point,
	which will be given in the `IntersectionPoint` parameter. If result is 2, the two segments overlap over another segment, which will
	be given in the `IntersectionSegment` parameter.
*/
inline int Intersect(segment2 S1, segment2 S2, V2* IntersectionPoint, segment2* IntersectionSegment) {
	if (AreParallel(S1, S2)) {
		V2 u = S1.Tail - S1.Head;
		V2 v = S2.Head - S1.Head;
		if (AreParallel(u, v)) {
			float R0 = dot(u, u);                 // Coordinates for S1.Tail
			float R1 = dot(u, v);                 // Coordinates for S2.Head
			float R2 = dot(u, S2.Tail - S1.Head); // Coordinates for S2.Tail

			// Swap if segments point in opposite directions
			bool Swap = R2 - R1 < 0;
			if (Swap) {
				float Temp = R1;
				R1 = R2;
				R2 = Temp;
				V2 Temp2 = S2.Head;
				S2.Head = S2.Tail;
				S2.Tail = Temp2;
			}

			if (fabsf(R1) < EPSILON) {
				if (R2 < EPSILON) {
					*IntersectionPoint = S2.Head;
					return 1;
				}
				else {
					*IntersectionSegment = S2;
					if (R2 > R0) {
						IntersectionSegment->Tail = S1.Tail;
					}
					return 2;
				}
			}
			else if (R1 < 0) {
				if (fabsf(R2) < EPSILON) {
					*IntersectionPoint = S2.Tail;
					return 1;
				}
				else if (R2 < 0) {
					return 0;
				}
				else {
					*IntersectionSegment = S1;
					if (R2 < R0) {
						IntersectionSegment->Tail = S2.Tail;
					}
					return 2;
				}
			}
			else {
				if (fabsf(R1 - R0) < EPSILON) {
					*IntersectionPoint = S2.Head;
					return 1;
				}
				else if (R1 > R0) {
					return 0;
				}
				else {
					IntersectionSegment->Head = S2.Head;
					IntersectionSegment->Tail = S1.Tail;
					return 2;
				}
			}
		}
		return 0;
	}
	else {
		V2 A = S1.Head;
		V2 B = S1.Tail;
		V2 C = S2.Head;
		V2 D = S2.Tail;

		float Orient1 = cross(B - A, C - A);
		float Orient2 = cross(B - A, D - A);
		float Orient3 = cross(D - C, A - C);
		float Orient4 = cross(D - C, B - C);

		if (Orient1 * Orient2 > 0 || Orient3 * Orient4 > 0) {
			return 0;
		}
		else {
			float t = - Orient1 / cross(B - A, D - C);
			Assert(t >= 0 && t <= 1);
			*IntersectionPoint = C + t * (D - C);
			return 1;
		}
	}
	return 0;
}

inline V2 ClosestPoint(segment2 Segment, V2 Point) {
	V2 V = Segment.Tail - Segment.Head;

	float t = dot(V, Point);
	if (t <= 0.0f) return Segment.Head;
	float Denom = dot(V, V);
	if (t >= Denom) return Segment.Tail;
	else return Segment.Head + (t / Denom) * V;
}

inline float SqDistance(segment2 Segment, V2 Point) {
	V2 V = Segment.Tail - Segment.Head;
	V2 R = Point - Segment.Head;

	R = project(R, V) - Point;
	return dot(R,R);
}

inline float SqDistance(segment2 Segment1, segment2 Segment2) {
	float d1 = SqDistance(Segment1, Segment2.Head);
	float d2 = SqDistance(Segment1, Segment2.Tail);
	float d3 = SqDistance(Segment2, Segment1.Head);
	float d4 = SqDistance(Segment2, Segment2.Tail);

	return fmin(d1, fmin(d2, fmin(d3, d4)));
}

INTROSPECT
struct Segment3 {
	V3 head;
	V3 tail;
};

inline Segment3 operator*(Transform transform, Segment3 segment) {
	return { transform * segment.head, transform * segment.tail };
}

/*
	Returns the transform that moves the segment [origin, (0,L,0)] to the input segment, L being the length of this segment.
*/
inline Transform SegmentTransform(Segment3 segment) {
	Transform result;
	result.scale = GetScale();
	result.translation = segment.head;
	V3 V = segment.tail - segment.head;
	float L = modulus(V);
	float angle = acosf(V.y / L);
	result.rotation = GetQuaternion(angle, V3(-V.z,0,V.x));
	return result;
}

inline V3 ClosestPoint(Segment3 segment, V3 point) {
	V3 V = segment.tail - segment.head;

	float t = dot(V, point);
	if (t <= 0.0f) return segment.head;
	float denom = dot(V, V);
	if (t >= denom) return segment.tail;
	else return segment.head + (t / denom) * V;
}

inline float SqDistance(Segment3 segment, V3 point) {
	V3 ab = segment.tail - segment.head;
	V3 ac = point - segment.head;
	V3 bc = point - segment.tail;
	float e = dot(ac, ab);
	// Handle cases where c projects outside ab
	if (e <= 0.0f) return dot(ac, ac);
	float f = dot(ab, ab);
	if (e >= f) return dot(bc, bc);
	// Handle cases where c projects onto ab
	return dot(ac, ac) - e * e / f;
}

inline float SqDistance(Segment3 segment1, Segment3 segment2) {
	V3 d1 = segment1.tail - segment1.head;
	V3 d2 = segment2.tail - segment2.head;
	V3 r = segment1.head - segment2.head;
	float a = dot(d1, d1);
	float b = dot(d1, d2);
	float c = dot(d2, d2);
	float d = dot(d1, r);
	float e = dot(d2, r);
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
	return dot(p1 - p2, p1 - p2);
}

inline float SqDistance(Segment3 segment, Ray ray) {
	V3 segment_direction = segment.tail - segment.head;
	V3 V = ray.point - segment.head;

    float b = dot(ray.direction, segment_direction);
    float c = dot(segment_direction, segment_direction);
    float d = dot(ray.direction, V);
    float e = dot(segment_direction, V);

    float denom = c - b * b;
    float t, u;

    if (denom != 0.0f) {
        t = (b * e - c * d) / denom;
        u = (e - b * d) / denom;
    } else {
        // Lines are parallel: pick t = 0, project S onto R
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
        u = dot(segment_direction, P - segment.head) / c;
        u = Clamp(u, 0.0f, 1.0f);
    }

    V3 D = V + t * ray.direction - u * segment_direction;
	return dot(D, D);
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

inline float GetArea(triangle2 T) {
	return 0.5f * cross(T.Points[2] - T.Points[0], T.Points[1] - T.Points[0]);
}

inline bool IsInside(triangle2 T, V2 P) {
	triangle2 U = { P, T.Points[1], T.Points[2] };
	triangle2 V = { P, T.Points[2], T.Points[0] };
	triangle2 W = { P, T.Points[0], T.Points[1] };

	float Area0 = GetArea(U);
	float Area1 = GetArea(V);
	float Area2 = GetArea(W);

	if (Area0 * Area1 > 0 && Area1 * Area2 > 0) {
		return true;
	}

	segment2 S1 = { T.Points[0], T.Points[1] };
	segment2 S2 = { T.Points[1], T.Points[2] };
	segment2 S3 = { T.Points[2], T.Points[0] };

	return IsInside(S1, P) || IsInside(S2, P) || IsInside(S3, P);
}

float SqDistance(triangle2 T, V2 P) {
	segment2 E[3] = {
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
			V = perp(T1.Points[i] - T1.Points[(i+1)%3]);
		}
		else {
			V = perp(T2.Points[i-3] - T2.Points[(i-2)%3]);
		}

		float c1[3] = {};
		float c2[3] = {};

		for (int j = 0; j < 3; j++) {
			c1[j] = dot(V, T1.Points[j]);
			c2[j] = dot(V, T2.Points[j]);
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

struct rectangle {
	float Left;
	float Top;
	float Width;
	float Height;
};

rectangle Rectangle(float Left, float Top, float Width, float Height) {
	return {Left, Top, Width, Height};
}

bool IsIn(rectangle Rect, V2 Position) {
	return Rect.Left <= Position.x && Position.x <= Rect.Left+Rect.Width &&
		   Rect.Top  <= Position.y && Position.y <= Rect.Top+Rect.Height;
}

V2 LeftTop(rectangle Rect) {
	return V2(Rect.Left, Rect.Top);
}

V2 Center(rectangle Rect) {
	return V2(Rect.Left + 0.5f * Rect.Width, Rect.Top  + 0.5f * Rect.Height);
}

bool Intersect(rectangle Rect1, rectangle Rect2) {
	V2 C1 = Center(Rect1);
	V2 C2 = Center(Rect2);
	return fabsf(C1.x - C2.x) < 0.5f * (Rect1.Width  + Rect2.Width) && 
	       fabsf(C1.y - C2.y) < 0.5f * (Rect1.Height + Rect2.Height);
}

struct vector_plane {
	V3 Normal;
};

vector_plane VectorPlane(V3 Normal) {
	return { normalize(Normal) };
}

struct affine_plane {
	float c; // The equation of the plane is dot(Normal, x) = c
	V3 Normal;
};

affine_plane AffinePlane(V3 Point, V3 Normal) {
	Normal = normalize(Normal);
	return { dot(Normal, Point), Normal };
}

inline V3 ClosestPoint(affine_plane Plane, V3 Point) {
	return Point - (dot(Point, Plane.Normal) - Plane.c) * Plane.Normal;
}

inline float Distance(affine_plane Plane, V3 Point) {
	return dot(Point, Plane.Normal) - Plane.c;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Colliders                                                                                                                                    |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

enum collider_type {
    Rect_Collider,
    Cube_Collider,
    Sphere_Collider,
	Capsule_Collider
};

struct collider {
    collider_type Type;
    V3 Offset;
    union {
		struct {
			float HalfWidth;
			float HalfHeight;
		} Rect;
		struct {
			float HalfWidth;
			float HalfHeight;
			float HalfDepth;
		} Cube;
		struct {
			float Radius;
		} Sphere;
		struct {
			Segment3 Segment;
			float Distance;
		} Capsule;
	};
};

collider RectCollider(V2 Offset, float Width, float Height) {
	collider result = {};
	result.Type = Rect_Collider;
	result.Offset = V3(Offset.x, Offset.y, 0);
	result.Rect.HalfWidth = 0.5f * Width;
	result.Rect.HalfHeight = 0.5f * Height;
	return result;
}

rectangle Rectangle(collider Collider) {
	Assert(Collider.Type == Rect_Collider);
	return {
		Collider.Offset.x - Collider.Rect.HalfWidth,
		Collider.Offset.y - Collider.Rect.HalfHeight,
		2.0f * Collider.Rect.HalfWidth,
		2.0f * Collider.Rect.HalfHeight
	};
}

collider CubeCollider(V3 Offset, float Width, float Height, float Depth) {
	collider result = {};
	result.Type = Cube_Collider;
	result.Offset = Offset;
	result.Cube.HalfWidth = 0.5f * Width;
	result.Cube.HalfHeight = 0.5f * Height;
	result.Cube.HalfDepth = 0.5f * Depth;
	return result;
}

collider SphereCollider(V3 Offset, float Radius) {
	collider result = {};
	result.Type = Sphere_Collider;
	result.Offset = Offset;
	result.Sphere.Radius = Radius;
	return result;
}

collider CapsuleCollider(V3 Head, V3 Tail, float Distance) {
	collider result = {};
	result.Type = Capsule_Collider;
	result.Offset = 0.5f * (Head + Tail);
	result.Capsule.Segment = {Head, Tail};
	result.Capsule.Distance = Distance;
	return result;
}

inline collider operator*(Transform T, collider C) {
	switch(C.Type) {
        case Rect_Collider: {
            V3 Displacement = T.translation;
			Displacement.z = 0;
			C.Offset += Displacement;
			return C;
        } break;

		case Sphere_Collider:
        case Cube_Collider: {
            V3 Displacement = T.translation;
			C.Offset += Displacement;
			return C;
        } break;

		case Capsule_Collider: {
			C.Capsule.Segment = T * C.Capsule.Segment;
			return C;
		} break;

		default: Assert(false);
	}
	return C;
}

bool Collide(collider Collider, V3 Position) {
    switch(Collider.Type) {
        case Rect_Collider: {
            return fabsf(Position.x - Collider.Offset.x) <= Collider.Rect.HalfWidth &&
                   fabsf(Position.y - Collider.Offset.y) <= Collider.Rect.HalfHeight;
        } break;

        case Cube_Collider: {
            return fabsf(Position.x - Collider.Offset.x) <= Collider.Cube.HalfWidth &&
                   fabsf(Position.y - Collider.Offset.y) <= Collider.Cube.HalfHeight &&
                   fabsf(Position.z - Collider.Offset.z) <= Collider.Cube.HalfDepth;
        } break;

        case Sphere_Collider: {
            return dot(Position - Collider.Offset, Position - Collider.Offset) <= Collider.Sphere.Radius * Collider.Sphere.Radius;
        } break;

		case Capsule_Collider: {
			return SqDistance(Collider.Capsule.Segment, Position) <= Collider.Capsule.Distance * Collider.Capsule.Distance;
		} break;

		default: Assert(false);
    }
    return false;
}

bool Collide(collider Collider1, collider Collider2) {
	if (Collider1.Type == Collider2.Type) {
		switch(Collider1.Type) {
			case Rect_Collider: {
				return Intersect(Rectangle(Collider1), Rectangle(Collider2));
			} break;
			case Cube_Collider: {
				float x[2] = { Collider1.Offset.x - Collider1.Cube.HalfWidth,  Collider1.Offset.x + Collider1.Cube.HalfWidth };
				float y[2] = { Collider1.Offset.y - Collider1.Cube.HalfHeight, Collider1.Offset.y + Collider1.Cube.HalfHeight };
				float z[2] = { Collider1.Offset.z - Collider1.Cube.HalfDepth,  Collider1.Offset.z + Collider1.Cube.HalfDepth };
				for (int i = 0; i < 2; i++) 
				for (int j = 0; j < 2; j++) 
				for (int k = 0; k < 2; k++) {
					if (Collide(Collider2, V3(x[i], y[j], z[k]))) return true;
				}
				return false;
			} break;
			case Sphere_Collider: {
				float SumRadii = Collider1.Sphere.Radius + Collider2.Sphere.Radius;
				V3 r = Collider2.Offset - Collider1.Offset;
				return dot(r, r) <= SumRadii * SumRadii;
			} break;
			case Capsule_Collider: {
				float SumDistance = Collider1.Capsule.Distance + Collider1.Capsule.Distance;
				return SqDistance(Collider1.Capsule.Segment, Collider2.Capsule.Segment) <= SumDistance * SumDistance;
			} break;
			default: Assert(false);
		}
	}
	else {
		if (Collider1.Type > Collider2.Type) {
			collider C = Collider2;
			Collider2 = Collider1;
			Collider1 = C;
		}
		Assert(Collider1.Type != Rect_Collider);
		switch (Collider1.Type) {
			case Cube_Collider: {
				Assert(false);
				if (Collider2.Type == Sphere_Collider) {

				}
				else if (Collider2.Type == Capsule_Collider) {

				}
			} break;

			case Sphere_Collider: {
				Assert(Collider2.Type == Capsule_Collider);
				float SumDistances = Collider1.Sphere.Radius + Collider2.Capsule.Distance;
				return SqDistance(Collider2.Capsule.Segment, Collider1.Offset) <= SumDistances * SumDistances;
			} break;

			default: Assert(false);
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

bool Raycast(Ray ray, collider Collider) {
	switch(Collider.Type) {
		case Rect_Collider: {
			return IsIn(Rectangle(Collider), V2(ray.point.x, ray.point.y));
		} break;
		case Cube_Collider: {
			float minB[3] = { 0 };
			minB[0] = Collider.Offset.x - Collider.Cube.HalfWidth / 2.0f;
			minB[1] = Collider.Offset.y - Collider.Cube.HalfHeight / 2.0f;
			minB[2] = Collider.Offset.z - Collider.Cube.HalfDepth / 2.0f;
			float maxB[3] = { 0 };
			maxB[0] = Collider.Offset.x + Collider.Cube.HalfWidth / 2.0f;
			maxB[1] = Collider.Offset.y + Collider.Cube.HalfHeight / 2.0f;
			maxB[2] = Collider.Offset.z + Collider.Cube.HalfDepth / 2.0f;
			float origin[3] = { ray.point.x, ray.point.y, ray.point.z };
			float dir[3] = { ray.direction.x, ray.direction.y, ray.direction.z };
			float coord[3] = { 0,0,0 };

			return HitBoundingBox(minB, maxB, origin, dir, coord);
		} break;
		case Sphere_Collider: {
			float R = Collider.Sphere.Radius;
			return SqDistance(ray, Collider.Offset) < R*R;
		} break;
		case Capsule_Collider: {
			float R = Collider.Capsule.Distance;
			return SqDistance(Collider.Capsule.Segment, ray) < R*R;
		} break;
		default: Assert(false);
	}
	return false;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Fast Fourier Transform                                                                                                                       |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

uint32 BitReverse(uint32 x) {
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
    x = (x << 16) | (x >> 16);
    return x;
}

uint32 BitReverse(uint32 x, uint32 log2N) {
	return BitReverse(x) >> (32 - log2N);
}

/*
	Takes an unsigned number and flips its digits around the decimal point to return a float in [0, 1).
	Example: BitReverseFloat of 3 (110 in binary) = 0.011 in binary = 0.75
*/
float BitReverseFloat(uint32 x) {
	uint32 FirstOne = log2(x);

	uint32 Mantissa = (BitReverse(x) << (1 + FirstOne)) >> 9;
	int8 Exponent = -FirstOne-1;

	float result = BuildFloat(false, Exponent, Mantissa);
	return result;
}

// Direct unoptimized calculation of Discrete Fourier Transform
void DFT(uint32 N, complex* Input, complex* Output) {
	for (int i = 0; i < N; i++) {
		complex result = Complex(0,0);
		for (int j = 0; j < N; j++) {
			result += Input[j]*expi(-TAU*j*i/N);
		}
		Output[i] = result;
	}
}

// Twiddle factors preparation
void PrepareTwiddleFactors(uint32 N, complex* Twiddle) {
	Twiddle[0] = Complex(1.0f, 0.0f);
	Twiddle[N >> 1] = Complex(-1.0f, 0.0f);
	Twiddle[N >> 2] = Complex(0.0f, -1.0f);
	Twiddle[3*(N >> 2)] = Complex(0.0f, 1.0f);

	for (int i = 0; i < N; i++) {
		if      (i == 0)          Twiddle[i] = Complex(1.0f, 0.0f);
		else if (i == N >> 1)     Twiddle[i] = Complex(-1.0f, 0.0f);
		else if (i == N >> 2)     Twiddle[i] = Complex(0.0f, -1.0f);
		else if (i == 3*(N >> 2)) Twiddle[i] = Complex(0.0f, 1.0f);
		else                      Twiddle[i] = expi(-TAU*i/N);
	}
}

void FFT_Butterfly(uint32 Stride, complex* Data, complex Twiddle) {
	complex x0 = Data[0];
	complex x1 = Data[Stride];

	Data[0]      = x0 + Twiddle*x1;
	Data[Stride] = x0 - Twiddle*x1;
}

void FFT(uint32 N, complex* Input, complex* Output, complex* Twiddle) {
	uint32 log2N = log2(N);
	
	// Bit reverse input
	for (int i = 0; i < N; i++) {
		Output[i] = Input[BitReverse(i, log2N)];
	}

	// FFT stages
	uint32 Stride = 1;
	uint32 TwiddleStride = N >> 1;
	for (int p = 0; p < log2N; p++) {		
		for (int Block = 0; Block < N; Block += Stride << 1)
		for (int k = 0; k < Stride; k++) {
			FFT_Butterfly(Stride, &Output[Block + k], Twiddle[k * TwiddleStride]);
		}
		
		Stride <<= 1;
		TwiddleStride >>= 1;
	}
}

#endif