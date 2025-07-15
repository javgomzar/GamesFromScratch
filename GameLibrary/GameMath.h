#ifndef GAME_MATH
#define GAME_MATH

#pragma once
#include "math.h"

//#include "fftw3.h"
//#pragma comment(lib, "libfftw3-3.lib")

#include "GamePlatform.h"

#ifndef INTROSPECT
#define INTROSPECT
#endif

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Utility macros                                                                                                                         |
// +----------------------------------------------------------------------------------------------------------------------------------------+

#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) ((uint64)Megabytes(Value)*1024)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Constants                                                                                                                              |
// +----------------------------------------------------------------------------------------------------------------------------------------+

const double Pi = 3.1415926535897932;
const double Tau = 6.2831853071795865;
const double twroot = 1.0594630943592952646;

const double Degrees = Pi / 180.0f;

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Arithmetic                                                                                                                             |
// +----------------------------------------------------------------------------------------------------------------------------------------+

inline int32 CustomRound(float X) {
	return X >= 0 ? (int32)(X + 0.5f) : (int32)(X - 0.5f);
}

inline int32 Sign(float x) {
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

inline float Clamp(float Value, float Min, float Max) {
	if (Value < Min) return Min;
	else if (Value > Max) return Max;
	else return Value;
}

inline int32 Clamp(int32 Value, int32 Min, int32 Max) {
	if (Value < Min) return Min;
	else if (Value > Max) return Max;
	else return Value;
}

inline uint32 Clamp(uint32 Value, uint32 Min, uint32 Max) {
	if (Value < Min) return Min;
	else if (Value > Max) return Max;
	else return Value;
}

/* FNV-1a hash algorithm */
inline uint32 Hash(void* Data, memory_index Size) {
	uint8* Bytes = (uint8*)Data;
	uint32 Hash = 2166136261u;
    for (memory_index i = 0; i < Size; i++) {
        Hash ^= Bytes[i];
        Hash *= 16777619u;
    }
    return Hash;
}

/* FNV-1a hash algorithm */
inline uint32 Hash(char* String) {
	uint32 Hash = 2166136261u;
    while (*String) {
        Hash ^= static_cast<uint8>(*String++);
        Hash *= 16777619u;
    }
    return Hash;
}

inline bool Bernoulli(float p = 0.5f) {
	return (float)rand() / (float)RAND_MAX >= p;
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | 2D                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct iv2 {
	int X, Y;
};

inline iv2 IV2(int X, int Y) {
	return { X, Y };
}

inline iv2 operator+(iv2 a, iv2 b) {
	return IV2(a.X + b.X, a.Y + b.Y);
}

inline bool operator==(const iv2& lhs, const iv2& rhs) {
	return lhs.X == rhs.X && lhs.Y == rhs.Y;
}

inline bool operator<(const iv2& lhs, const iv2& rhs) {
	bool Result = 0;
	if (lhs.X == rhs.X) {
		if (lhs.Y == rhs.Y) {
			return false;
		}
		else return lhs.Y < rhs.Y;
	}
	else return lhs.X < rhs.X;
}

struct v2 {
	float X, Y;
};

inline v2 V2(float X, float Y) {
	v2 Result;
	Result.X = X;
	Result.Y = Y;
	return Result;
}

inline v2 Radial(float Angle) {
	return V2(cosf(Angle), sinf(Angle));
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

inline v2 operator*(float C, v2 A) {
	v2 Result;
	Result.X = C * A.X;
	Result.Y = C * A.Y;
	return Result;
}

inline v2 operator*(v2 A, v2 B) {
	return { A.X * B.X , A.Y * B.Y };
}

inline v2& operator+=(v2& A, v2 B) {
	A.X += B.X;
	A.Y += B.Y;
	return A;
}

inline v2& operator/=(v2& A, float C) {
	A.X /= C;
	A.Y /= C;
	return A;
}

inline v2& operator-=(v2& A, v2 B) {
	A.X -= B.X;
	A.Y -= B.Y;
	return A;
}

inline v2& operator*=(v2& A, float C) {
	A.X *= C;
	A.Y *= C;
	return A;
}

inline float dot(v2 A, v2 B) {
	return A.X * B.X + A.Y * B.Y;
}

inline float modulus(v2 A) {
	return sqrt(dot(A, A));
}

inline v2 normalize(v2 V) {
	return (modulus(V) < 0.00001f) ? V : (1 / modulus(V)) * V;
}

inline v2 project(v2 A, v2 B) {
	v2 N = normalize(B);
	return (A * N) * N;
}
 
inline v2 perp(v2 A) {
	return { -A.Y, A.X };
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | 3D                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct uv3 {
	uint32 X, Y, Z;
};

inline uv3 UV3(uint32 X, uint32 Y, uint32 Z) {
	return { X, Y, Z };
}

inline uv3 operator+(uv3 a, uv3 b) {
	return UV3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
}

inline bool operator==(const uv3& lhs, const uv3& rhs) {
	return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z;
}

struct iv3 {
	int X, Y, Z;
};

inline iv3 IV3(int X, int Y, int Z) {
	return { X, Y, Z };
}

inline iv3 operator+(iv3 a, iv3 b) {
	return IV3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
}

inline bool operator==(const iv3& lhs, const iv3& rhs) {
	return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z;
}

inline bool operator<(const iv3& lhs, const iv3& rhs) {
	bool Result = 0;
	if (lhs.X == rhs.X) {
		if (lhs.Y == rhs.Y) {
			if (lhs.Z == rhs.Z) return false;
			else return lhs.Z < rhs.Z;
		}
		else return lhs.Y < rhs.Y;
	}
	else return lhs.X < rhs.X;
}

struct v3 {
	float X, Y, Z;
};

inline v3 V3(float X, float Y, float Z) {
	return { X, Y, Z };
}

inline v3 V3(v2 XY, float Z) {
	return { XY.X, XY.Y, Z};
};

inline v3 V3(float X, v2 YZ) {
	return { X, YZ.X, YZ.Y};
}

inline v3 V3(iv3 V) {
	return V3(V.X, V.Y, V.Z);
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

inline v3 operator*(float C, v3 A) {
	v3 Result;
	Result.X = C * A.X;
	Result.Y = C * A.Y;
	Result.Z = C * A.Z;
	return Result;
}

 inline v3 operator*(v3 A, float C) {
	v3 Result;
	Result.X = C * A.X;
	Result.Y = C * A.Y;
	Result.Z = C * A.Z;
	return Result;
}

 inline v3 operator*(v3 A, v3 B) {
	 return V3(A.X * B.X, A.Y * B.Y, A.Z * B.Z);
 }

inline v3 operator/(v3 A, float C) {
	v3 Result;
	Result.X = A.X / C;
	Result.Y = A.Y / C;
	Result.Z = A.Z / C;
	return Result;
}

inline v3& operator+=(v3& A, v3 B) {
	A.X += B.X;
	A.Y += B.Y;
	A.Z += B.Z;
	return A;
}

inline v3& operator/=(v3& A, float C) {
	A.X /= C;
	A.Y /= C;
	A.Z /= C;
	return A;
}

inline v3& operator-=(v3& A, v3 B) {
	A.X -= B.X;
	A.Y -= B.Y;
	A.Z -= B.Z;
	return A;
}

inline v3& operator*=(v3& A, float C) {
	A.X *= C;
	A.Y *= C;
	A.Z *= C;
	return A;
}

inline bool operator==(v3 A, v3 B) {
	return A.X == B.X && A.Y == B.Y && A.Z == B.Z;
}

inline bool operator!=(v3 A, v3 B) {
	return A.X != B.X || A.Y != B.Y || A.Z != B.Z;
}

inline float dot(v3 A, v3 B) {
	return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
}

inline v3 cross(v3 A, v3 B) {
	v3 Result;
	Result.X = A.Y * B.Z - A.Z * B.Y;
	Result.Y = A.Z * B.X - A.X * B.Z;
	Result.Z = A.X * B.Y - A.Y * B.X;
	return Result;
}

inline float modulus(v3 A) {
	return sqrt(dot(A, A));
}

inline float distance(v3 A, v3 B) {
	return modulus(B - A);
}

inline v3 normalize(v3 V) {
	return (modulus(V) < 0.00001) ? V : (1 / modulus(V)) * V;
}

inline v3 project(v3 A, v3 B) {
	return dot(A, B) * B / dot(B, B);
}

inline v3 DirectionFromAngle(float Theta) {
	v3 Result;
	Result.X = cosf(Theta);
	Result.Y = sinf(Theta);
	Result.Z = 0;
	return Result;
}

inline v3 SphereNormal(float Theta, float Phi) {
	v3 Result;
	Result.X = cosf(Phi)*sinf(Theta);
	Result.Y = sinf(Phi)*sinf(Theta);
	Result.Z = cosf(Theta);
	return Result;
}

inline v3 Rotate(v3 v, float Angle) {
	float X = v.X * cosf(Angle) + v.Y * sinf(Angle);
	float Y = -v.X * sinf(Angle) + v.Y * cosf(Angle);
	float Z = v.Z;
	return V3(X, Y, Z);
}

inline v3 Rotate(v3 v, v3 w, float Angle = 0.0) {
	if (Angle == 0.0) {
		float Angle = Tau * modulus(w);
	}
	v3 n = normalize(w);
	// Rodrigues formula
	return cosf(Angle) * v + sinf(Angle) * cross(n, v) + (1 - cosf(Angle)) * (n * v) * n;
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | 4D                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct iv4 {
	int X, Y, Z, W;
};

inline iv4 IV4(int X, int Y, int Z, int W) {
	return { X, Y, Z, W };
}

inline iv4 operator+(iv4 a, iv4 b) {
	return IV4(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
}

inline bool operator==(const iv4& lhs, const iv4& rhs) {
	return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z;
}

inline bool operator<(const iv4& lhs, const iv4& rhs) {
	bool Result = 0;
	if (lhs.X == rhs.X) {
		if (lhs.Y == rhs.Y) {
			if (lhs.Z == rhs.Z) return false;
			else return lhs.Z < rhs.Z;
		}
		else return lhs.Y < rhs.Y;
	}
	else return lhs.X < rhs.X;
}

struct v4 {
	float X, Y, Z, W;
};

inline v4 V4(float X, float Y, float Z, float W) {
	return { X, Y, Z, W };
}

inline v4 V4(v3 V, float W) {
	return { V.X, V.Y, V.Z, W };
}

inline v4 V4(iv4 V) {
	return V4(V.X, V.Y, V.Z, V.W);
}

inline v4 operator+(v4 A, v4 B) {
	v4 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	Result.Z = A.Z + B.Z;
	Result.W = A.W + B.W;
	return Result;
}

inline v4 operator-(v4 A) {
	v4 Result;
	Result.X = -A.X;
	Result.Y = -A.Y;
	Result.Z = -A.Z;
	Result.W = -A.W;
	return Result;
}

inline v4 operator-(v4 A, v4 B) {
	v4 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	Result.Z = A.Z - B.Z;
	Result.W = A.W - B.W;
	return Result;
}

inline v4 operator*(float C, v4 A) {
	v4 Result;
	Result.X = C * A.X;
	Result.Y = C * A.Y;
	Result.Z = C * A.Z;
	Result.W = C * A.W;
	return Result;
}

 inline v4 operator*(v4 A, float C) {
	v4 Result;
	Result.X = C * A.X;
	Result.Y = C * A.Y;
	Result.Z = C * A.Z;
	Result.W = C * A.W;
	return Result;
}

 inline v4 operator*(v4 A, v4 B) {
	 return V4(A.X * B.X, A.Y * B.Y, A.Z * B.Z, A.W * B.W);
 }

inline v4& operator+=(v4& A, v4 B) {
	A.X += B.X;
	A.Y += B.Y;
	A.Z += B.Z;
	A.W += B.W;
	return A;
}

inline v4& operator-=(v4& A, v4 B) {
	A.X -= B.X;
	A.Y -= B.Y;
	A.Z -= B.Z;
	A.W -= B.W;
	return A;
}

inline v4& operator*=(v4& A, float C) {
	A.X *= C;
	A.Y *= C;
	A.Z *= C;
	A.W *= C;
	return A;
}

inline float dot(v4 A, v4 B) {
	return A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Matrices                                                                                                                               |
// +----------------------------------------------------------------------------------------------------------------------------------------+

union matrix2 {
	struct {
		float XX,XY,
		      YX,YY;
	};
	float Array[4];
	struct {
		v2 Row[2];
	};
	struct {
		v2 X, Y;;
	};
	struct {
		float Element[2][2];
	};
};

matrix2 Identity2 = {
	1,0,
	0,1
};

inline v2 col(matrix2 A, int i) {
	switch(i) {
		case 0: return V2(A.XX, A.YX); break;
		case 1: return V2(A.XY, A.YY); break;
		default: Assert(false); return V2(0,0);
	};
}

inline matrix2 transpose(matrix2 A) {
	matrix2 Result;
	Result.X = col(A, 0);
	Result.Y = col(A, 1);

	return Result;
}

inline matrix2 operator*(float c, matrix2 A) {
	matrix2 Result = A;
	Result.X *= c;
	Result.Y *= c;
	return Result;
}

inline matrix2 operator-(matrix2 A) {
	matrix2 Result = A;
	Result.X = -Result.X;
	Result.Y = -Result.Y;
	return Result;
}

inline matrix2 operator*(matrix2 A, matrix2 B) {
	matrix2 Result;
	Result.XX = A.XX * B.XX + A.XY * B.YX;
	Result.XY = A.XX * B.XY + A.XY * B.YY;
	Result.YX = A.YX * B.XX + A.YY * B.YX;
	Result.YY = A.YX * B.XY + A.YY * B.YY;
	return Result;
}

inline v2 operator*(matrix2 A, v2 V) {
	v2 Result;
	Result.X = dot(A.X, V);
	Result.Y = dot(A.Y, V);
	return Result;
}

inline v2 operator*(v2 V, matrix2 A) {
	v2 Result;
	Result.X = dot(V, col(A, 0));
	Result.Y = dot(V, col(A, 1));
	return Result;
}

inline matrix2 operator/(matrix2 A, float c) {
	matrix2 Result = A;
	Result.X /= c;
	Result.Y /= c;
	return Result;
}

inline matrix2& operator*=(matrix2& A, float c) {
	A.X *= c;
	A.Y *= c;
	return A;
}

inline bool operator==(matrix2 A, matrix2 B) {
	for (int i = 0; i < 4; i++) {
		if (fabs(A.Array[i] - B.Array[i]) > 0.001f) return false;
	}
	return true;
}

inline float det(matrix2 A) {
	return A.XX * A.YY - A.XY * A.YX;
}

inline matrix2 inverse(matrix2 A) {
	float D = det(A);
	Assert(fabs(D) > 0.001f);
	matrix2 Result = {
		A.YY / D, -A.XY / D,
		-A.YX / D, A.XX / D
	};
	return Result;
}

union matrix3 {
	struct {
		float XX,XY,XZ,
		       YX,YY,YZ,
			   ZX,ZY,ZZ;
	};
	float Array[9];
	struct {
		v3 Row[3];
	};
	struct {
		v3 X, Y, Z;
	};
	struct {
		float Element[3][3];
	};
};

matrix3 Identity3 = {
	1,0,0,
	0,1,0,
	0,0,1
};

inline v3 col(matrix3 A, int i) {
	switch(i) {
		case 0: return V3(A.XX, A.YX, A.ZX); break;
		case 1: return V3(A.XY, A.YY, A.ZY); break;
		case 2: return V3(A.XZ, A.YZ, A.ZZ); break;
		default: Assert(false); return V3(0,0,0);
	};
}

inline matrix3 transpose(matrix3 A) {
	matrix3 Result;
	Result.X = col(A, 0);
	Result.Y = col(A, 1);
	Result.Z = col(A, 2);

	return Result;
}

inline matrix3 operator*(float c, matrix3 A) {
	matrix3 Result = A;
	Result.X *= c;
	Result.Y *= c;
	Result.Z *= c;
	return Result;
}

inline matrix3 operator-(matrix3 A) {
	matrix3 Result = A;
	Result.X = -Result.X;
	Result.Y = -Result.Y;
	Result.Z = -Result.Z;
	return Result;
}

inline matrix3 operator*(matrix3 A, matrix3 B) {
	matrix3 Result;
	Result.XX = dot(A.X, col(B, 0));
	Result.XY = dot(A.X, col(B, 1));
	Result.XZ = dot(A.X, col(B, 2));
	Result.YX = dot(A.Y, col(B, 0));
	Result.YY = dot(A.Y, col(B, 1));
	Result.YZ = dot(A.Y, col(B, 2));
	Result.ZX = dot(A.Z, col(B, 0));
	Result.ZY = dot(A.Z, col(B, 1));
	Result.ZZ = dot(A.Z, col(B, 2));
	return Result;
}

inline v3 operator*(matrix3 A, v3 V) {
	v3 Result;
	Result.X = dot(A.X, V);
	Result.Y = dot(A.Y, V);
	Result.Z = dot(A.Z, V);
	return Result;
}

inline v3 operator*(v3 V, matrix3 A) {
	v3 Result;
	Result.X = dot(V, col(A, 0));
	Result.Y = dot(V, col(A, 1));
	Result.Z = dot(V, col(A, 2));
	return Result;
}

inline matrix3 operator/(matrix3 A, float c) {
	matrix3 Result = A;
	Result.X /= c;
	Result.Y /= c;
	Result.Z /= c;
	return Result;
}

inline matrix3& operator*=(matrix3& A, float c) {
	A.X *= c;
	A.Y *= c;
	A.Z *= c;
	return A;
}

inline bool operator==(matrix3 A, matrix3 B) {
	for (int i = 0; i < 9; i++) {
		if (fabs(A.Array[i] - B.Array[i]) > 0.001f) return false;
	}
	return true;
}

inline float det(matrix3 A) {
	return A.XX * A.YY * A.ZZ + A.XY * A.YZ * A.ZX + A.XZ * A.YX * A.ZY - A.XZ * A.YY * A.ZX  - A.XY * A.YX * A.ZZ - A.XX * A.YZ * A.ZY;
}

inline matrix3 adjugate(matrix3 A) {
	matrix3 Result;
	Result.XX = A.YY * A.ZZ - A.YZ * A.ZY;
	Result.XY = A.YZ * A.ZX - A.YX * A.ZZ;
	Result.XZ = A.YX * A.ZY - A.YY * A.ZX;
	Result.YX = A.XZ * A.ZY - A.XY * A.ZZ;
	Result.YY = A.XX * A.ZZ - A.XZ * A.ZX;
	Result.YZ = A.XY * A.ZX - A.XX * A.ZY;
	Result.ZX = A.XY * A.YZ - A.XZ * A.YY;
	Result.ZY = A.XZ * A.YX - A.XX * A.YZ;
	Result.ZZ = A.XX * A.YY - A.XY * A.YX;
	return Result;
}

inline matrix3 inverse(matrix3 A) {
	Assert(fabs(det(A)) > 0.01);
	return adjugate(A) / det(A);
}

inline matrix3 Rotation(float Angle, v3 Axis) {
	v3 u = normalize(Axis);

	float cos_ = cosf(Angle);
	float sin_ = sinf(Angle);

	matrix3 Result;
	Result.XX = cos_ + u.X * u.X * (1 - cos_);
	Result.XY = u.X * u.Y * (1 - cos_) - u.Z * sin_;
	Result.XZ = u.X * u.Z * (1 - cos_) + u.Y * sin_;
	Result.YX = u.Y * u.X * (1 - cos_) + u.Z * sin_;
	Result.YY = cos_ + u.Y * u.Y * (1 - cos_);
	Result.YZ = u.Y * u.Z * (1 - cos_) - u.X * sin_;
	Result.ZX = u.Z * u.X * (1 - cos_) - u.Y * sin_;
	Result.ZY = u.Z * u.Y * (1 - cos_) + u.X * sin_;
	Result.ZZ = cos_ + u.Z * u.Z * (1 - cos_);
	return Result;
}

typedef matrix3 basis;

inline v3 ChangeBasis(v3 V, basis Basis) {
	return V.X * Basis.X + V.Y * Basis.Y + V.Z * Basis.Z;
}

inline basis Rotate(basis Basis, float Angle) {
	basis Result;
	Result.X = Rotate(Basis.X, Angle);
	Result.Y = Rotate(Basis.Y, Angle);
	Result.Z = Rotate(Basis.Z, Angle);
	return Result;
}

inline basis Rotate(basis Basis, v3 w) {
	basis Result;
	Result.X = Rotate(Basis.X, w);
	Result.Y = Rotate(Basis.Y, w);
	Result.Z = Rotate(Basis.Z, w);
	return Result;
}

inline basis Complete(v3 X, v3 Y) {
	basis Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = normalize(cross(X,Y));
	return Result;
}

inline basis Complete(v3 X) {
	v3 Y = cross(X, V3(1,0,0));
	if (modulus(Y) < 0.01) {
		Y = cross(X, V3(0,1,0));
		if (modulus(Y) < 0.01) {
			Y = cross(X, V3(0,0,1));
		}
	}
	return Complete(X, normalize(Y));
}

inline basis Identity(float Factor = 1.0) {
	basis Result;
	Result.X = V3(Factor, 0.0, 0.0);
	Result.Y = V3(0.0, Factor, 0.0);
	Result.Z = V3(0.0, 0.0, Factor);
	return Result;
}

inline basis normalize(basis Basis) {
	basis Result;
	Result.X = normalize(Basis.X);
	Result.Y = normalize(Basis.Y);
	Result.Z = normalize(Basis.Z);
	return Result;
}

union matrix4 {
	struct {
		float XX,XY,XZ,XW,
		       YX,YY,YZ,YW,
			   ZX,ZY,ZZ,ZW,
			   WX,WY,WZ,WW;
	};
	float Array[16];
	struct {
		v4 Row[4];
	};
	struct {
		v4 X, Y, Z, W;
	};
	struct {
		float Element[4][4];
	};
};

matrix4 Identity4 = {
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};

inline v4 col(matrix4 A, int i) {
	switch(i) {
		case 0: return V4(A.XX, A.YX, A.ZX, A.WX); break;
		case 1: return V4(A.XY, A.YY, A.ZY, A.WY); break;
		case 2: return V4(A.XZ, A.YZ, A.ZZ, A.WZ); break;
		case 3: return V4(A.XW, A.YW, A.ZW, A.WW); break;
		default: Assert(false); return V4(0,0,0,0);
	};
}

inline matrix4 transpose(matrix4 A) {
	matrix4 Result;
	Result.X = col(A, 0);
	Result.Y = col(A, 1);
	Result.Z = col(A, 2);
	Result.W = col(A, 3);
	return Result;
}

inline matrix4 operator*(matrix4 A, matrix4 B) {
	matrix4 Result;
	Result.XX = dot(A.X, col(B, 0));
	Result.XY = dot(A.X, col(B, 1));
	Result.XZ = dot(A.X, col(B, 2));
	Result.XW = dot(A.X, col(B, 3));
	Result.YX = dot(A.Y, col(B, 0));
	Result.YY = dot(A.Y, col(B, 1));
	Result.YZ = dot(A.Y, col(B, 2));
	Result.YW = dot(A.Y, col(B, 3));
	Result.ZX = dot(A.Z, col(B, 0));
	Result.ZY = dot(A.Z, col(B, 1));
	Result.ZZ = dot(A.Z, col(B, 2));
	Result.ZW = dot(A.Z, col(B, 3));
	Result.WX = dot(A.W, col(B, 0));
	Result.WY = dot(A.W, col(B, 1));
	Result.WZ = dot(A.W, col(B, 2));
	Result.WW = dot(A.W, col(B, 3));
	return Result;
}

inline v4 operator*(matrix4 A, v4 V) {
	v4 Result;
	Result.X = dot(A.X, V);
	Result.Y = dot(A.Y, V);
	Result.Z = dot(A.Z, V);
	Result.W = dot(A.W, V);
	return Result;
}

inline bool operator==(matrix4 A, matrix4 B) {
	for (int i = 0; i < 16; i++) {
		if (fabs(A.Element[i] - B.Element[i]) > 0.001f) return false;
	}
	return true;
}

inline matrix3 Matrix3(matrix4 Matrix) {
	return {
		Matrix.XX, Matrix.XY, Matrix.XZ,
		Matrix.YX, Matrix.YY, Matrix.YZ,
		Matrix.ZX, Matrix.ZY, Matrix.ZZ
	};
}

inline matrix4 Matrix4(matrix3 Matrix) {
	matrix4 Result;
	Result.X = V4(Matrix.X, 0);
	Result.Y = V4(Matrix.Y, 0);
	Result.Z = V4(Matrix.Z, 0);
	Result.W = V4(0,0,0,1);
	return Result;
}

matrix4 GetScreenProjectionMatrix(float Width, float Height) {
	float a = 2.0f / Width;
	float b = 2.0f / Height;

	matrix4 Result = {
		   a, 0.0f, 0.0f, 0.0f,
		0.0f,   -b, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
	   -1.0f, 1.0f, 0.0f, 1.0f,
	};

	return Result;
}

matrix4 GetWorldProjectionMatrix(float Width, float Height) {
	float sX = 1.0;
	float sY = Width / Height;
	float sZ = 1.0;

	matrix4 Result = {
		  sX, 0.0f, 0.0f, 0.0f,
		0.0f,   sY, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,   sZ,
		0.0f, 0.0f,-1.0f, 0.0f,
	};
	
	return Result;
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Complex numbers                                                                                                                        |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct complex {
	double r; // Real part
	double i; // Imaginary part
};

complex Complex(double r, double i) {
	return {r, i};
}

inline complex conjugate(complex A) {
	return {
		A.r,
		-A.i
	};
}

inline complex operator+(complex A, double B) {
	return {
		A.r + B,
		A.i
	};
}

inline complex& operator+=(complex& A, double B) {
	A.r += B;
	return A;
}

inline complex operator+(double A, complex B) {
	return {
		A + B.r,
		B.i
	};
}

inline complex operator-(double A, complex B) {
	return {
		A - B.r,
		B.i
	};
}

inline complex operator-(complex A, double B) {
	return {
		A.r - B,
		A.i
	};
}

inline complex& operator-=(complex& A, double B) {
	A.r -= B;
	return A;
}

inline complex operator*(complex A, double C) {
	return {
		A.r * C,
		A.i * C
	};
}

inline complex operator*(double C, complex A) {
	return {
		A.r * C,
		A.i * C
	};
}

inline complex& operator*=(complex& A, double B) {
	A.r *= B;
	A.i *= B;
	return A;
}

inline complex operator/(complex A, double C) {
	return {
		A.r / C,
		A.i / C
	};
}

inline complex& operator/=(complex& A, double B) {
	A.r /= B;
	A.i /= B;
	return A;
}

inline double modulus(complex A) {
	return sqrt(A.r * A.r + A.i * A.i);
}

inline complex inverse(complex A) {
	return conjugate(A) / (A.r * A.r + A.i * A.i);
}

inline complex expi(double Alpha) {
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

inline complex operator/(double A, complex B) {
	return A * inverse(B);
}

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Quaternions                                                                                                                            |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct quaternion {
	float c;
	float i;
	float j;
	float k;
};

const quaternion I = { 0.0, 1.0, 0.0, 0.0 };
const quaternion J = { 0.0, 0.0, 1.0, 0.0 };
const quaternion K = { 0.0, 0.0, 0.0, 1.0 };

inline quaternion Quaternion(float c, float i = 0.0, float j = 0.0, float k = 0.0) {
	return { c, i, j, k };
}

inline quaternion Quaternion(float Angle, v3 Vector) {
	float sin_angle = sinf(Angle / 2);
	Vector = normalize(Vector);
	return { cosf(Angle / 2.0), sin_angle * Vector.X, sin_angle * Vector.Y, sin_angle * Vector.Z};
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

inline quaternion operator*(float c, quaternion q) {
	return { c * q.c, c * q.i, c * q.j, c * q.k};
}

inline quaternion operator*(quaternion q, float c) {
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

inline v3 operator*(quaternion Q, v3 Vector) {
	quaternion Q_ = Conjugate(Q);
	quaternion V = Quaternion(0.0, Vector.X, Vector.Y, Vector.Z);
	quaternion Result = Q_ * V * Q;
	return V3(Result.i, Result.j, Result.k);
}

inline basis operator*(quaternion Q, basis Basis) {
	basis Result;
	Result.X = Q * Basis.X;
	Result.Y = Q * Basis.Y;
	Result.Z = Q * Basis.Z;

	return Result;
}

inline bool operator==(quaternion Q1, quaternion Q2) {
	return Q1.c == Q2.c && Q1.i == Q2.i && Q1.j == Q2.j && Q1.k == Q2.k; 
}

inline bool operator!=(quaternion Q1, quaternion Q2) {
	return Q1.c != Q2.c || Q1.i == Q2.i || Q1.j == Q2.j || Q1.k == Q2.k; 
}

inline matrix3 Matrix(quaternion Q) {
	matrix3 Result;
	Result.XX  = (2.0f * (Q.c * Q.c + Q.i * Q.i) - 1.0f);
	Result.XY  = 2.0f * (Q.i * Q.j - Q.c * Q.k);
	Result.XZ  = 2.0f * (Q.i * Q.k + Q.c * Q.j);
	Result.YX  = 2.0f * (Q.i * Q.j + Q.c * Q.k);
	Result.YY  = (2.0f * (Q.c * Q.c + Q.j * Q.j) - 1.0f);
	Result.YZ  = 2.0f * (Q.j * Q.k - Q.c * Q.i);
	Result.ZX  = 2.0f * (Q.i * Q.k - Q.c * Q.j);
	Result.ZY  = 2.0f * (Q.j * Q.k + Q.c * Q.i);
	Result.ZZ = (2.0f * (Q.c * Q.c + Q.k * Q.k) - 1.0f);
	return Result;
}


// +----------------------------------------------------------------------------------------------------------------------------------------+
// | Transform                                                                                                                              |
// +----------------------------------------------------------------------------------------------------------------------------------------+

struct scale {
	float X;
	float Y;
	float Z;
};

inline scale Scale(float X = 1.0, float Y = 1.0, float Z = 1.0) {
	return { X, Y, Z };
};

inline scale operator*(float C, scale S) {
	return { C * S.X, C * S.Y, C * S.Z };
}

inline scale operator*(scale S, scale T) {
	return Scale(S.X * T.X, S.Y * T.Y, S.Z * T.Z);
}

inline v3 operator*(scale Scale, v3 Vector) {
	return V3(Scale.X * Vector.X, Scale.Y * Vector.Y, Scale.Z * Vector.Z);
}

inline basis operator*(scale Scale, basis Basis) {
	basis Result;
	Result.X = Scale.X * Basis.X;
	Result.Y = Scale.Y * Basis.Y;
	Result.Z = Scale.Z * Basis.Z;
	return Result;
}

inline bool operator==(scale Scale1, scale Scale2) {
	return Scale1.X == Scale2.X && Scale1.Y == Scale2.Y && Scale1.Z == Scale2.Z;
}

inline bool operator!=(scale Scale1, scale Scale2) {
	return Scale1.X != Scale2.X || Scale1.Y != Scale2.Y || Scale1.Z != Scale2.Z;
}

INTROSPECT
struct transform {
	v3 Translation;
	scale Scale;
	quaternion Rotation;
};

transform IdentityTransform = { V3(0,0,0), Scale(), Quaternion(1.0f) };

inline transform Transform(quaternion Rotation, v3 Translation = V3(0.0, 0.0, 0.0), scale Scaling = Scale()) {
	return { Translation, Scaling, Rotation };
}

inline transform Transform(v3 Translation, quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0), scale Scaling = Scale()) {
	return { Translation, Scaling, Rotation };
}

inline v3 operator*(transform T, v3 Vector) {
	return T.Rotation * (T.Scale * Vector) + T.Translation;
}

inline v4 operator*(transform T, v4 Vector) {
	return V4(T.Rotation * (T.Scale * V3(Vector.X, Vector.Y, Vector.Z)) + Vector.W * T.Translation, Vector.W);
}

inline basis operator*(transform T, basis Basis) {
	return T.Rotation * Basis;
}

inline transform operator*(transform T, transform U) {
	transform Result = { 0 };
	Result.Scale = T.Scale * U.Scale;
	Result.Translation = U.Translation + U.Rotation * T.Translation;
	Result.Rotation = T.Rotation * U.Rotation;
	return Result;
}

inline bool operator==(transform T, transform U) {
	return T.Translation == U.Translation && T.Scale == U.Scale && T.Rotation == U.Rotation;
}

inline bool operator!=(transform T, transform U) {
	return T.Translation != U.Translation || T.Scale == U.Scale || T.Rotation == U.Rotation;
}

inline matrix4 Matrix(transform T) {
	matrix3 Rotation = Matrix(T.Rotation);
	matrix4 Result;
	Result.X = T.Scale.X * V4(Rotation.X,0);
	Result.Y = T.Scale.Y * V4(Rotation.Y,0);
	Result.Z = T.Scale.Z * V4(Rotation.Z,0);
	Result.W = V4(T.Translation, 1.0f);
	return Result;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Geometry                                                                                                                                     |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

struct segment {
	v3 Head;
	v3 Tail;
};

inline segment operator*(transform T, segment S) {
	return { T * S.Head, T * S.Tail };
}

/*
	Returns the transform that moves the segment [origin, (0,L,0)] to the input segment, L being the length of this segment.
*/
inline transform SegmentTransform(segment Segment) {
	transform Result;
	Result.Translation = Segment.Head;
	v3 V = Segment.Tail - Segment.Head;
	float L = modulus(V);
	float Angle = acosf(V.Y / L);
	Result.Rotation = Quaternion(Angle, V3(-V.Z,0,V.X));
	return Result;
}

inline v3 ClosestPoint(segment Segment, v3 Point) {
	v3 V = Segment.Tail - Segment.Head;

	float t = dot(V, Point);
	if (t <= 0.0f) return Segment.Head;
	float Denom = dot(V, V);
	if (t >= Denom) return Segment.Tail;
	else return Segment.Head + (t / Denom) * V;
}

inline float SqDistance(segment Segment, v3 Point) {
	v3 ab = Segment.Tail - Segment.Head;
	v3 ac = Point - Segment.Head;
	v3 bc = Point - Segment.Tail;
	float e = dot(ac, ab);
	// Handle cases where c projects outside ab
	if (e <= 0.0f) return dot(ac, ac);
	float f = dot(ab, ab);
	if (e >= f) return dot(bc, bc);
	// Handle cases where c projects onto ab
	return dot(ac, ac) - e * e / f;
}

inline float SqDistance(segment Segment1, segment Segment2) {
	v3 d1 = Segment1.Tail - Segment1.Head;
	v3 d2 = Segment2.Tail - Segment2.Head;
	v3 r = Segment1.Head - Segment2.Head;
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

	v3 p1 = Segment1.Head + s * d1;
	v3 p2 = Segment2.Head + t * d2;
	return dot(p1 - p2, p1 - p2);
}

struct ray {
	v3 Point;
	v3 Direction;
};

ray Ray(v3 Point, v3 Direction) {
	return { Point, normalize(Direction) };
}

inline v3 ClosestPoint(ray Ray, v3 Point) {
	float t = dot(Ray.Direction, Point - Ray.Point);
	if (t <= 0.0f) return Ray.Point;
	else return Ray.Point + t * Ray.Direction;
}

inline float SqDistance(ray Ray, v3 Point) {
	v3 D = Point - ClosestPoint(Ray, Point);
	return dot(D,D);
}

inline ray MouseRay(float Width, float Height, v3 CameraPosition, basis CameraBasis, v2 Mouse) {
	v3 Direction =
        (2.0 * Mouse.X / Width - 1.0) *    CameraBasis.X +
        (Height - 2.0 * Mouse.Y) / Width * CameraBasis.Y - 
                                           CameraBasis.Z;
	ray Result = Ray(CameraPosition, Direction);
	return Result;
}

inline float SqDistance(segment Segment, ray Ray) {
	v3 SegmentDirection = Segment.Tail - Segment.Head;
	v3 V = Ray.Point - Segment.Head;

    float b = dot(Ray.Direction, SegmentDirection);
    float c = dot(SegmentDirection, SegmentDirection);
    float d = dot(Ray.Direction, V);
    float e = dot(SegmentDirection, V);

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

    // Clamp t to [0, ∞) for the ray
    if (t < 0.0f) {
        t = 0.0f;
        // Recompute u for this t
        v3 P = Ray.Point + t * Ray.Direction;
        u = dot(SegmentDirection, P - Segment.Head) / c;
        u = Clamp(u, 0.0f, 1.0f);
    }

    v3 D = V + t * Ray.Direction - u * SegmentDirection;
	return modulus(D);
}

struct line {
	v3 Point;
	v3 Direction;
};

line Line(v3 Point, v3 Direction) {
	return { Point, normalize(Direction) };
}

line LineFromPoints(v3 A, v3 B) {
	return { A, normalize(B - A) };
}

inline v3 ClosestPoint(line Line, v3 Point) {
	float t = dot(Line.Direction, Point - Line.Point);
	return Point + t * Line.Direction;
}

inline float Distance(line Line, v3 Point) {
	return dot(Line.Direction, Point - Line.Point);
}

inline float Distance(line Line1, line Line2) {
	float a = dot(Line1.Direction, Line2.Direction);
	if (a == 0.0) { // Lines are parallel
		return Distance(Line1, Line2.Point);
	}
	else {
		v3 v = Line1.Point - Line2.Point;
		float b1 = dot(Line1.Direction, v);
		float b2 = dot(Line2.Direction, v);
		float s = (a * b2 - b1) / a;
		float t = (b2 - a * b1) / a;
		v3 Point1 = Line1.Point + s * Line1.Point;
		v3 Point2 = Line2.Point + t * Line2.Direction;
		return modulus(Point1 - Point2);
	}
}

struct triangle {
	v3 Points[3];
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

bool IsIn(rectangle Rect, v2 Position) {
	return Rect.Left <= Position.X && Position.X <= Rect.Left+Rect.Width &&
		   Rect.Top  <= Position.Y && Position.Y <= Rect.Top+Rect.Height;
}

v2 LeftTop(rectangle Rect) {
	return V2(Rect.Left, Rect.Top);
}

struct vector_plane {
	v3 Normal;
};

vector_plane VectorPlane(v3 Normal) {
	return { normalize(Normal) };
}

struct affine_plane {
	float c; // The equation of the plane is dot(Normal, x) = c
	v3 Normal;
};

affine_plane AffinePlane(v3 Point, v3 Normal) {
	Normal = normalize(Normal);
	return { dot(Normal, Point), Normal };
}

inline v3 ClosestPoint(affine_plane Plane, v3 Point) {
	return Point - (dot(Point, Plane.Normal) - Plane.c) * Plane.Normal;
}

inline float Distance(affine_plane Plane, v3 Point) {
	return dot(Point, Plane.Normal) - Plane.c;
}

//void Orthogonalize(vector_plane* Plane) {
//	Plane->Base[0] = normalize(Plane->Base[0]);
//	Plane->Base[1] -= dot(Plane->Base[0], Plane->Base[1]) * Plane->Base[0];
//}

//v3 Project(v3 Vector, vector_plane Plane) {
//	Orthogonalize(&Plane);
//	return dot(Vector, Plane.Base[0]) * Plane.Base[0] + dot(Vector, Plane.Base[1]) * Plane.Base[1];
//}

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
    v3 Offset;
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
			segment Segment;
			float Distance;
		} Capsule;
	};
};

collider RectCollider(v2 Offset, float Width, float Height) {
	collider Result = {};
	Result.Type = Rect_Collider;
	Result.Offset = V3(Offset.X, Offset.Y, 0);
	Result.Rect.HalfWidth = 0.5f * Width;
	Result.Rect.HalfHeight = 0.5f * Height;
	return Result;
}

rectangle Rectangle(collider Collider) {
	Assert(Collider.Type == Rect_Collider);
	return {
		Collider.Offset.X - Collider.Rect.HalfWidth,
		Collider.Offset.Y - Collider.Rect.HalfHeight,
		2.0f * Collider.Rect.HalfWidth,
		2.0f * Collider.Rect.HalfHeight
	};
}

collider CubeCollider(v3 Offset, float Width, float Height, float Depth) {
	collider Result = {};
	Result.Type = Cube_Collider;
	Result.Offset = Offset;
	Result.Cube.HalfWidth = 0.5f * Width;
	Result.Cube.HalfHeight = 0.5f * Height;
	Result.Cube.HalfDepth = 0.5f * Depth;
	return Result;
}

collider SphereCollider(v3 Offset, float Radius) {
	collider Result = {};
	Result.Type = Sphere_Collider;
	Result.Offset = Offset;
	Result.Sphere.Radius = Radius;
	return Result;
}

collider CapsuleCollider(v3 Head, v3 Tail, float Distance) {
	collider Result = {};
	Result.Type = Capsule_Collider;
	Result.Offset = 0.5f * (Head + Tail);
	Result.Capsule.Segment = {Head, Tail};
	Result.Capsule.Distance = Distance;
	return Result;
}

inline collider operator*(transform T, collider C) {
	switch(C.Type) {
        case Rect_Collider: {
            v3 Displacement = T.Translation;
			Displacement.Z = 0;
			C.Offset += Displacement;
			return C;
        } break;

		case Sphere_Collider:
        case Cube_Collider: {
            v3 Displacement = T.Translation;
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

bool Collide(collider Collider, v3 Position) {
    switch(Collider.Type) {
        case Rect_Collider: {
            return fabs(Position.X - Collider.Offset.X) <= Collider.Rect.HalfWidth &&
                   fabs(Position.Y - Collider.Offset.Y) <= Collider.Rect.HalfHeight;
        } break;

        case Cube_Collider: {
            return fabs(Position.X - Collider.Offset.X) <= Collider.Cube.HalfWidth &&
                   fabs(Position.Y - Collider.Offset.Y) <= Collider.Cube.HalfHeight &&
                   fabs(Position.Z - Collider.Offset.Z) <= Collider.Cube.HalfDepth;
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
				float Left   = Collider1.Offset.X - Collider1.Rect.HalfWidth;
				float Right  = Collider1.Offset.X + Collider1.Rect.HalfWidth;
				float Top    = Collider1.Offset.Y - Collider1.Rect.HalfHeight;
				float Bottom = Collider1.Offset.Y + Collider1.Rect.HalfHeight;
				v2 Points[4] = { V2(Left, Top), V2(Left, Bottom), V2(Right, Top), V2(Right, Bottom) };
				for (int i = 0; i < 4; i++) {
					if (Collide(Collider2, V3(Points[i], 0))) return true;
				}
				return false;
			} break;
			case Cube_Collider: {
				float X[2] = { Collider1.Offset.X - Collider1.Cube.HalfWidth,  Collider1.Offset.X + Collider1.Cube.HalfWidth };
				float Y[2] = { Collider1.Offset.Y - Collider1.Cube.HalfHeight, Collider1.Offset.Y + Collider1.Cube.HalfHeight };
				float Z[2] = { Collider1.Offset.Z - Collider1.Cube.HalfDepth,  Collider1.Offset.Z + Collider1.Cube.HalfDepth };
				for (int i = 0; i < 2; i++) 
				for (int j = 0; j < 2; j++) 
				for (int k = 0; k < 2; k++) {
					if (Collide(Collider2, V3(X[i], Y[j], Z[k]))) return true;
				}
				return false;
			} break;
			case Sphere_Collider: {
				float SumRadii = Collider1.Sphere.Radius + Collider2.Sphere.Radius;
				v3 r = Collider2.Offset - Collider1.Offset;
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
    register int i;
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

bool Raycast(ray Ray, collider Collider) {
	switch(Collider.Type) {
		case Rect_Collider: {
			return IsIn(Rectangle(Collider), V2(Ray.Point.X, Ray.Point.Y));
		} break;
		case Cube_Collider: {
			float minB[3] = { 0 };
			minB[0] = Collider.Offset.X - Collider.Cube.HalfWidth / 2.0f;
			minB[1] = Collider.Offset.Y - Collider.Cube.HalfHeight / 2.0f;
			minB[2] = Collider.Offset.Z - Collider.Cube.HalfDepth / 2.0f;
			float maxB[3] = { 0 };
			maxB[0] = Collider.Offset.X + Collider.Cube.HalfWidth / 2.0f;
			maxB[1] = Collider.Offset.Y + Collider.Cube.HalfHeight / 2.0f;
			maxB[2] = Collider.Offset.Z + Collider.Cube.HalfDepth / 2.0f;
			float origin[3] = { Ray.Point.X, Ray.Point.Y, Ray.Point.Z };
			float dir[3] = { Ray.Direction.X, Ray.Direction.Y, Ray.Direction.Z };
			float coord[3] = { 0,0,0 };

			return HitBoundingBox(minB, maxB, origin, dir, coord);
		} break;
		case Sphere_Collider: {
			float R = Collider.Sphere.Radius;
			return SqDistance(Ray, Collider.Offset) < R*R;
		} break;
		case Capsule_Collider: {
			float R = Collider.Capsule.Distance;
			return SqDistance(Collider.Capsule.Segment, Ray) < R*R;
		} break;
		default: Assert(false);
	}
	return false;
}

#endif