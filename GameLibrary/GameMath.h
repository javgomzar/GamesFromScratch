#ifndef GAME_MATH
#define GAME_MATH

#pragma once
#include "math.h"

//#include "fftw3.h"
//#pragma comment(lib, "libfftw3-3.lib")

#include "../GameLibrary/GamePlatform.h"

// Utility macros
#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) ((uint64)Megabytes(Value)*1024)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

// Constants
const double Pi = 3.1415926535897932;
const double Tau = 6.2831853071795865;
const double twroot = 1.0594630943592952646;

const double Degrees = Pi / 180.0f;

// Arithmetics
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

// Vectors
// 2D
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

// 3D
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
	v3 N = normalize(B);
	return (A * N) * N;
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

// 4D
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

// Matrices
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
	return c * A;
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

// Complex numbers
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

// Quaternions
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

inline v3 Rotate(v3 Vector, quaternion Q) {
	quaternion Q_ = Conjugate(Q);
	quaternion V = Quaternion(0.0, Vector.X, Vector.Y, Vector.Z);
	quaternion Result = Q_ * V * Q;
	return V3(Result.i, Result.j, Result.k);
}

inline basis Rotate(basis Basis, quaternion Q) {
	basis Result;
	Result.X = Rotate(Basis.X, Q);
	Result.Y = Rotate(Basis.Y, Q);
	Result.Z = Rotate(Basis.Z, Q);

	return Result;
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

inline transform Transform(v3 Translation = V3(0.0, 0.0, 0.0), quaternion Rotation = Quaternion(1.0, 0.0, 0.0, 0.0), scale Scaling = Scale()) {
	return { Translation, Scaling, Rotation };
}

inline v3 operator*(transform Transform, v3 Vector) {
	return Rotate((Transform.Scale * Vector), Transform.Rotation) + Transform.Translation;
}

inline transform operator*(transform T, transform U) {
	transform Result = { 0 };
	Result.Scale = T.Scale * U.Scale;
	Result.Translation = U.Translation + Rotate(T.Translation, U.Rotation);
	Result.Rotation = T.Rotation * U.Rotation;
	return Result;
}

inline matrix4 Matrix(transform Transform) {
	matrix4 Result;
	Result.Array[0]  = Transform.Scale.X * (2.0 * (Transform.Rotation.c * Transform.Rotation.c + Transform.Rotation.i * Transform.Rotation.i) - 1.0);
	Result.Array[1]  = Transform.Scale.X * 2.0 * (Transform.Rotation.i * Transform.Rotation.j - Transform.Rotation.c * Transform.Rotation.k);
	Result.Array[2]  = Transform.Scale.X * 2.0 * (Transform.Rotation.i * Transform.Rotation.k + Transform.Rotation.c * Transform.Rotation.j);
	Result.Array[3]  = 0.0;
	Result.Array[4]  = Transform.Scale.Y * 2.0 * (Transform.Rotation.i * Transform.Rotation.j + Transform.Rotation.c * Transform.Rotation.k);
	Result.Array[5]  = Transform.Scale.Y * (2.0 * (Transform.Rotation.c * Transform.Rotation.c + Transform.Rotation.j * Transform.Rotation.j) - 1.0);
	Result.Array[6]  = Transform.Scale.Y * 2.0 * (Transform.Rotation.j * Transform.Rotation.k - Transform.Rotation.c * Transform.Rotation.i);
	Result.Array[7]  = 0.0;
	Result.Array[8]  = Transform.Scale.Z * 2.0 * (Transform.Rotation.i * Transform.Rotation.k - Transform.Rotation.c * Transform.Rotation.j);
	Result.Array[9]  = Transform.Scale.Z * 2.0 * (Transform.Rotation.j * Transform.Rotation.k + Transform.Rotation.c * Transform.Rotation.i);
	Result.Array[10] = Transform.Scale.Z * (2.0 * (Transform.Rotation.c * Transform.Rotation.c + Transform.Rotation.k * Transform.Rotation.k) - 1.0);
	Result.Array[11] = 0.0;
	Result.Array[12] = Transform.Translation.X;;
	Result.Array[13] = Transform.Translation.Y;
	Result.Array[14] = Transform.Translation.Z;
	Result.Array[15] = 1.0;
	return Result;
}

// Geometry
struct game_triangle {
	v3 Points[3];
};

struct game_rect {
	float Left;
	float Top;
	float Width;
	float Height;
};

struct vector_plane {
	v3 Base[2];
};

struct affine_plane {
	v3 Position;
	union {
		vector_plane VectorPlane;
		v3 Base[2];
	};
};

//void Orthogonalize(vector_plane* Plane) {
//	Plane->Base[0] = normalize(Plane->Base[0]);
//	Plane->Base[1] -= dot(Plane->Base[0], Plane->Base[1]) * Plane->Base[0];
//}

//v3 Project(v3 Vector, vector_plane Plane) {
//	Orthogonalize(&Plane);
//	return dot(Vector, Plane.Base[0]) * Plane.Base[0] + dot(Vector, Plane.Base[1]) * Plane.Base[1];
//}


#endif