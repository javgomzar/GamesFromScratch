#include "pch.h"
#include "mathematics.h"

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| Arithmetic                                                                                                                             |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

int32 Round(float x) {
	return x >= 0.0f ? (int32)(x + 0.5f) : (int32)(x - 0.5f);
}

int32 Sign(float x) {
	if      (x > 0) return 1;
	else if (x < 0) return -1;
	return 0;
}

float Clamp(float value, float min, float max) {
	if      (value < min) return min;
	else if (value > max) return max;
	return value;
}

int32 Clamp(int32 value, int32 min, int32 max) {
	if      (value < min) return min;
	else if (value > max) return max;
	return value;
}

uint32 Clamp(uint32 value, uint32 min, uint32 max) {
	if      (value < min) return min;
	else if (value > max) return max;
	return value;
}

// Makes an angle be in the [0, 360) interval.
float NormalizeAngle(float angle) {
	if (angle >= 360) {
		return angle - 360;
	}
	else if (angle < 0) {
		return angle + 360;
	}
	return angle;
}

int8 GetExponent32(float number) {
	uint32 Bytes;
	memcpy(&Bytes, &number, 4);
	return ((Bytes & 0x7F800000) >> 23) - 127;
}

uint32 GetMantissa32(float number) {
	uint32 Bytes;
	memcpy(&Bytes, &number, 4);
	return Bytes & 0x7FFFFF;
}

int16 GetExponent64(double number) {
	uint64 Bytes;
	memcpy(&Bytes, &number, 8);
	return ((Bytes & 0x7FF0000000000000ULL) >> 52) - 1023;
}

uint64 GetMantissa64(double number) {
	uint64 Bytes;
	memcpy(&Bytes, &number, 8);
	return Bytes & 0xFFFFFFFFFFFFFULL;
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

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| RNG                                                                                                                                    |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

void RNG::NextState() {
	state ^= state << 13;
	state ^= state >> 7;
	state ^= state << 17;
}

uint64 Seed() {
#ifdef _DEBUG
    time_t seconds = time(nullptr);
    tm* local_time = localtime(&seconds);
    uint64 seed = local_time->tm_mday + 123456789;
#else
    uint64 seed = Platform::GetWallClock();
#endif
    for (int i = 0; i < 8; i++) {
        seed ^= seed << 13;
		seed ^= seed >> 7;
		seed ^= seed << 17;
    }
    return seed;
}

const uint64 RNG::seed = Seed();
uint64 RNG::state = RNG::seed;

float RNG::RandFloat() {
	float result = (float)state / (float)UINT64_MAX;
	RNG::NextState();
	return result;
}

float RNG::RandFloat(float min, float max) {
	assert(min <= max);
	return RandFloat() * (max - min) + min;
}

bool RNG::Bernoulli(float p = 0.5f) {
	return RandFloat() >= p;
}

// Returns a random integer greater or equal than min and strictly under max.
int RNG::RandInt(int min, int max) {
	return RandFloat() * (max - min) + min;
}

float RNG::Normal() {
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

float RNG::Normal(float Mean, float StdDeviation) {
	return Mean + StdDeviation * Normal();
}

void RNG::LogSeed(Arena& transient) {
	String message = String::Format(transient, "RNG seed: {U}.", 1, seed);
	Log::Info(message);
}

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| 2D                                                                                                                                     |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

V2::V2(float X, float Y) : x(X), y(Y) {}

V2 V2::operator-() {
	return V2(-x, -y);
}

V2& V2::operator+=(V2 rhs) {
	x += rhs.x;
	y += rhs.y;
	return *this;
}

V2& V2::operator-=(V2 rhs) {
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

V2& V2::operator*=(float rhs) {
	x *= rhs;
	y *= rhs;
	return *this;
}

V2& V2::operator/=(float rhs) {
	x /= rhs;
	y /= rhs;
	return *this;
}

V2 operator+(V2 lhs, V2 rhs) {
	return V2(lhs.x + rhs.x, lhs.y + rhs.y);
}

V2 operator-(V2 lhs, V2 rhs) {
	return V2(lhs.x - rhs.x, lhs.y - rhs.y);
}

V2 operator*(float lhs, V2 rhs) {
	return V2(lhs * rhs.x, lhs * rhs.x);
}

V2 operator*(V2 lhs, V2 rhs) {
	return V2(lhs.x * rhs.x , lhs.y * rhs.y);
}

bool operator==(V2 lhs, V2 rhs) {
	return fabsf(lhs.x - rhs.x) < EPSILON && fabsf(lhs.y - rhs.y) < EPSILON;
}

bool operator!=(V2 lhs, V2 rhs) {
	return fabsf(lhs.x - rhs.x) >= EPSILON || fabsf(lhs.y - rhs.y) >= EPSILON;
}

V2 Radial(float angle) {
	return V2(cosf(angle), sinf(angle));
}

float Dot(V2 lhs, V2 rhs) {
	return lhs.x * rhs.x + lhs.y * rhs.y;
}

float Cross(V2 lhs, V2 rhs) {
	return lhs.x * rhs.y - lhs.y * rhs.x;
}

bool AreParallel(V2 lhs, V2 rhs) {
	float f = Cross(lhs, rhs);
	return fabsf(f) < EPSILON;
}

float Modulus(V2 v) {
	return sqrt(Dot(v, v));
}

float GetAngle(V2 lhs, V2 rhs) {
	float m1 = Modulus(lhs);
	float m2 = Modulus(rhs);
	if (m1 < EPSILON || m2 < EPSILON) {
		return 0;
	}
	float s = Cross(lhs, rhs) / m1 / m2;
	float c = Dot(lhs, rhs) / m1 / m2;
	return atan2f(s, c);
}

V2 Normalize(V2 v) {
	return (Modulus(v) < EPSILON) ? V2(0,0) : (1 / Modulus(v)) * v;
}

// Projects `lhs` into `rhs`.
V2 Project(V2 lhs, V2 rhs) {
	V2 n = Normalize(rhs);
	return Dot(lhs, n) * n;
}

float Distance(V2 lhs, V2 rhs) {
	return Modulus(lhs - rhs);
}
 
V2 Perpendicular(V2 v) {
	return V2(-v.y, v.x);
}

UV2::UV2(uint32 X, uint32 Y) : x(X), y(Y) {}

UV2 operator+(UV2 lhs, UV2 rhs) {
	return UV2(lhs.x + rhs.x, lhs.y + rhs.y);
}

bool operator==(const UV2& lhs, const UV2& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

IV2::IV2(int X, int Y) : x(X), y(Y) {}

IV2 operator+(IV2 lhs, IV2 rhs) {
	return IV2(lhs.x + rhs.x, lhs.y + rhs.y);
}

bool operator==(IV2 lhs, IV2 rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator<(IV2 lhs, IV2 rhs) {
	if (lhs.x == rhs.x) {
		if (lhs.y == rhs.y) {
			return false;
		}
		else return lhs.y < rhs.y;
	}
	else return lhs.x < rhs.x;
}

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| 3D                                                                                                                                     |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

V3::V3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
V3::V3(float X, V2 V) : x(X), y(V.x), z(V.y) {}
V3::V3(V2 V, float Z) : x(V.x), y(V.y), z(Z) {}

V3 V3::operator-() {
	return V3(-x, -y, -z);
}

V3& V3::operator+=(V3 rhs) {
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

V3& V3::operator-=(V3 rhs) {
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

V3& V3::operator*=(float rhs) {
	x *= rhs;
	y *= rhs;
	z *= rhs;
	return *this;
}

V3& V3::operator/=(float rhs) {
	x /= rhs;
	y /= rhs;
	z /= rhs;
	return *this;
}

V3 operator+(V3 lhs, V3 rhs) {
	return V3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

V3 operator-(V3 lhs, V3 rhs) {
	return V3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

V3 operator*(float lhs, V3 rhs) {
	return V3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

V3 operator*(V3 lhs, V3 rhs) {
	return V3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

V3 operator/(V3 lhs, float rhs) {
	return V3(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}

bool operator==(V3 lhs, V3 rhs) {
	return fabsf(lhs.x - rhs.x) < EPSILON && fabsf(lhs.y - rhs.y) < EPSILON && fabsf(lhs.z - rhs.z) < EPSILON;
}

bool operator!=(V3 lhs, V3 rhs) {
	return fabsf(lhs.x - rhs.x) >= EPSILON || fabsf(lhs.y - rhs.y) >= EPSILON || fabsf(lhs.z - rhs.z) >= EPSILON;
}

V3 Radial(float angle, float pitch) {
	float cosA = cosf(angle * DEGREES);
    float sinA = sinf(angle * DEGREES);
    float cosP = cosf(pitch * DEGREES);
    float sinP = sinf(pitch * DEGREES);
    return {sinA * cosP, sinP, cosA * cosP};
}

float Dot(V3 lhs, V3 rhs) {
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

V3 Cross(V3 lhs, V3 rhs) {
	return V3(lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x);
}

float Modulus(V3 v) {
	return sqrt(Dot(v, v));
}

float Distance(V3 lhs, V3 rhs) {
	return Modulus(rhs - lhs);
}

V3 Normalize(V3 v) {
	return (Modulus(v) < EPSILON) ? V3(0,0,0) : (1.0f / Modulus(v)) * v;
}

V3 Project(V3 lhs, V3 rhs) {
	return Dot(lhs, rhs) * rhs / Dot(rhs, rhs);
}

V3 DirectionFromAngle(float angle) {
	return V3(Radial(angle), 0);
}

V3 SphereNormal(float theta, float phi) {
	return V3(cosf(phi)*sinf(theta), sinf(phi)*sinf(theta), cosf(theta));
}

V3 Rotate(V3 v, float angle) {
	return V3(
		v.x * cosf(angle) + v.y * sinf(angle), 
		-v.x * sinf(angle) + v.y * cosf(angle), 
		v.z
	);
}

V3 Rotate(V3 v, V3 w, float angle = 0.0) {
	if (angle == 0.0) {
		angle = TAU * Modulus(w);
	}
	V3 n = Normalize(w);
	// Rodrigues formula
	return cosf(angle) * v + sinf(angle) * Cross(n, v) + (1 - cosf(angle)) * (n * v) * n;
}

UV3::UV3(uint32 X, uint32 Y, uint32 Z) : x(X), y(Y), z(Z) {}

UV3 operator+(UV3 lhs, UV3 rhs) {
	return UV3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

bool operator==(UV3 lhs, UV3 rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

IV3::IV3(int X, int Y, int Z) : x(X), y(Y), z(Z) {}

IV3 operator+(IV3 lhs, IV3 rhs) {
	return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

bool operator==(IV3 lhs, IV3 rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

bool operator<(IV3 lhs, IV3 rhs) {
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

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| 4D                                                                                                                                     |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

V4::V4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}
V4::V4(V3 V, float W) : x(V.x), y(V.y), z(V.z), w(W) {}
V4::V4(V2 U, V2 V) : x(U.x), y(U.y), z(V.x), w(V.y) {}
V4::V4(float X, V3 V) : x(X), y(V.x), z(V.y), w(V.z) {}

V4 V4::operator-() {
	return V4(-x, -y, -z, -w);
}

V4& V4::operator+=(V4 rhs) {
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;
	return *this;
}

V4& V4::operator-=(V4 rhs) {
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w -= rhs.w;
	return *this;
}

V4& V4::operator*=(float rhs) {
	x *= rhs;
	y *= rhs;
	z *= rhs;
	w *= rhs;
	return *this;
}

V4& V4::operator/=(float rhs) {
	x /= rhs;
	y /= rhs;
	z /= rhs;
	w /= rhs;
	return *this;
}

V4 operator+(V4 lhs, V4 rhs) {
	return V4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}

V4 operator-(V4 lhs, V4 rhs) {
	return V4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}

V4 operator*(V4 lhs, V4 rhs) {
	return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w};
}

V4 operator*(float lhs, V4 rhs) {
	return V4(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
}

V4 operator/(V4 lhs, float rhs) {
	return V4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
}

float Dot(V4 lhs, V4 rhs) {
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

IV4::IV4(int X, int Y, int Z, int W) : x(X), y(Y), z(Z), w(W) {}

IV4 operator+(IV4 lhs, IV4 rhs) {
	return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w};
}

bool operator==(IV4 lhs, IV4 rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

bool operator<(IV4 lhs, IV4 rhs) {
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

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| Matrices                                                                                                                               |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

Matrix2::Matrix2(float XX, float XY, float YX, float YY) : xx(XX), xy(XY), yx(YX), yy(YY) {}
Matrix2::Matrix2(V2 X, V2 Y) : x(X), y(Y) {}

const Matrix2 Matrix2::ID = Matrix2(
	1, 0,
	0, 1
);

Matrix2 Matrix2::operator-() {
	return Matrix2(-x, -y);
}

Matrix2& Matrix2::operator*=(float f) {
	xx *= f;
	xy *= f;
	yx *= f;
	yy *= f;
	return *this;
}

V2 Matrix2::Col(int i) {
	switch(i) {
		case 0: return V2(xx, yx); break;
		case 1: return V2(xy, yy); break;
	};
	Log::Fatal("Invalid column index for Matrix2.");
	return V2(0, 0);
}

Matrix2 Matrix2::Transpose() {
	return Matrix2(Col(0), Col(1));
}

Matrix2 operator*(float lhs, Matrix2 rhs) {
	return Matrix2(
		lhs * rhs.xx, lhs * rhs.xy,
		lhs * rhs.yx, lhs * rhs.yy
	);
}

Matrix2 operator/(Matrix2 lhs, float rhs) {
	return Matrix2(
		lhs.xx / rhs, lhs.xy / rhs,
		lhs.yx / rhs, lhs.yy / rhs
	);
}

V2 operator*(Matrix2 lhs, V2 rhs) {
	return V2(Dot(lhs.x, rhs), Dot(lhs.y, rhs));
}

V2 operator*(V2 lhs, Matrix2 rhs) {
	return V2(Dot(lhs, rhs.Col(0)), Dot(lhs, rhs.Col(1)));
}

Matrix2 operator*(Matrix2 lhs, Matrix2 rhs) {
	return Matrix2(
		lhs.xx * rhs.xx + lhs.xy * rhs.yx, lhs.xx * rhs.xy + lhs.xy * rhs.yy,
		lhs.yx * rhs.xx + lhs.yy * rhs.yx, lhs.yx * rhs.xy + lhs.yy * rhs.yy
	);
}

bool operator==(Matrix2 lhs, Matrix2 rhs) {
	for (int i = 0; i < 4; i++) {
		if (fabsf(lhs.array[i] - rhs.array[i]) > EPSILON) return false;
	}
	return true;
}

float Matrix2::Det() {
	return xx * yy - xy * yx;
}

Matrix2 Matrix2::Inverse() {
	float d = Det();
	assert(fabsf(d) > EPSILON);
	Matrix2 result = {
		yy / d, -xy / d,
		-yx / d, xx / d
	};
	return result;
}

Matrix3::Matrix3(
	float XX, float XY, float XZ,
    float YX, float YY, float YZ,
    float ZX, float ZY, float ZZ
) : 
	xx(XX), xy(XY), xz(XZ),
	yx(YX), yy(YY), yz(YZ),
	zx(ZX), zy(ZY), zz(ZZ)
{}

Matrix3::Matrix3(V3 X, V3 Y, V3 Z) : x(X), y(Y), z(Z) {}

const Matrix3 Matrix3::ID = Matrix3(
	1, 0, 0,
	0, 1, 0,
	0, 0, 1
);

Matrix3 Matrix3::operator-() {
	return Matrix3(
		-xx,-xy,-xz,
		-yx,-yy,-yz,
		-zx,-zy,-zz
	);
}

Matrix3& Matrix3::operator*=(float f) {
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

V3 Matrix3::Col(int i) {
	switch(i) {
		case 0: { return V3(xx, yx, zx); } break;
		case 1: { return V3(xy, yy, zy); } break;
		case 2: { return V3(xz, yz, zz); } break;
	};
	Log::Fatal("Invalid column index for Matrix3.");
	return V3(0, 0, 0);
}

Matrix3 Matrix3::Transpose() {
	return Matrix3(Col(0), Col(1), Col(2));
}

float Matrix3::Det() {
	return xx * yy * zz + xy * yz * zx + xz * yx * zy - xz * yy * zx - xy * yx * zz - xx * yz * zy;
}

Matrix3 Matrix3::Adjugate() {
	return Matrix3(
		yy * zz - yz * zy, yz * zx - yx * zz, yx * zy - yy * zx,
		xz * zy - xy * zz, xx * zz - xz * zx, xy * zx - xx * zy,
		xy * yz - xz * yy, xz * yx - xx * yz, xx * yy - xy * yx
	);
}

Matrix3 Matrix3::Inverse() {
	float d = Det();
	assert(fabsf(d) > EPSILON);
	return Adjugate() / d;
}

Matrix3 operator*(float lhs, Matrix3 rhs) {
	return Matrix3(
		lhs * rhs.xx, lhs * rhs.xy, lhs * rhs.xz,
		lhs * rhs.yx, lhs * rhs.yy, lhs * rhs.yz,
		lhs * rhs.zx, lhs * rhs.zy, lhs * rhs.zz
	);
}

Matrix3 operator/(Matrix3 lhs, float rhs) {
	return Matrix3(
		lhs.xx / rhs, lhs.xy / rhs, lhs.xz / rhs,
		lhs.yx / rhs, lhs.yy / rhs, lhs.yz / rhs,
		lhs.zx / rhs, lhs.zy / rhs, lhs.zz / rhs
	);
}

V3 operator*(Matrix3 lhs, V3 rhs) {
	return V3(Dot(lhs.x, rhs), Dot(lhs.y, rhs), Dot(lhs.z, rhs));
}

V3 operator*(V3 lhs, Matrix3 rhs) {
	return V3(Dot(lhs, rhs.Col(0)), Dot(lhs, rhs.Col(1)), Dot(lhs, rhs.Col(2)));
}

Matrix3 operator*(Matrix3 lhs, Matrix3 rhs) {
	return Matrix3(
		Dot(lhs.x, rhs.Col(0)), Dot(lhs.x, rhs.Col(1)), Dot(lhs.x, rhs.Col(2)),
		Dot(lhs.y, rhs.Col(0)), Dot(lhs.y, rhs.Col(1)), Dot(lhs.y, rhs.Col(2)),
		Dot(lhs.z, rhs.Col(0)), Dot(lhs.z, rhs.Col(1)), Dot(lhs.z, rhs.Col(2))
	);
}

bool operator==(Matrix3 lhs, Matrix3 rhs) {
	for (int i = 0; i < 9; i++) {
		if (fabsf(lhs.array[i] - rhs.array[i]) >= EPSILON) return false;
	}
	return true;
}

Matrix3 Rotation(float angle, V3 axis) {
	V3 u = Normalize(axis);

	float c = cosf(angle);
	float s = sinf(angle);

	return Matrix3(
		c + u.x * u.x * (1 - c), u.x * u.y * (1 - c) - u.z * s, u.x * u.z * (1 - c) + u.y * s,
		u.y * u.x * (1 - c) + u.z * s, c + u.y * u.y * (1 - c), u.y * u.z * (1 - c) - u.x * s,
		u.z * u.x * (1 - c) - u.y * s, u.z * u.y * (1 - c) + u.x * s, c + u.z * u.z * (1 - c)
	);
}

V3 ChangeBasis(V3 v, Basis basis) {
	return v.x * basis.x + v.y * basis.y + v.z * basis.z;
}

Basis Rotate(Basis basis, float angle) {
	return Basis(
		Rotate(basis.x, angle),
		Rotate(basis.y, angle),
		Rotate(basis.z, angle)
	);
}

Basis Rotate(Basis basis, V3 w) {
	return Basis(
		Rotate(basis.x, w),
		Rotate(basis.y, w),
		Rotate(basis.z, w)
	);
}

Basis Complete(V3 x, V3 y) {
	return Basis(x, y, Normalize(Cross(x, y)));
}

Basis Complete(V3 x) {
	V3 y = Cross(x, V3(1,0,0));
	if (Modulus(y) < EPSILON) {
		y = Cross(x, V3(0,1,0));
		if (Modulus(y) < EPSILON) {
			y = Cross(x, V3(0,0,1));
		}
	}
	return Complete(x, Normalize(y));
}

Basis Normalize(Basis basis) {
	return Basis(
		Normalize(basis.x),
		Normalize(basis.y),
		Normalize(basis.z)
	);
}

Matrix4::Matrix4(
	float XX, float XY, float XZ, float XW,
	float YX, float YY, float YZ, float YW,
	float ZX, float ZY, float ZZ, float ZW,
	float WX, float WY, float WZ, float WW
) : 
	xx(XX), xy(XY), xz(XZ), xw(XW),
	yx(YX), yy(YY), yz(YZ), yw(YW),
	zx(ZX), zy(ZY), zz(ZZ), zw(ZW)
{}

Matrix4::Matrix4(V4 X, V4 Y, V4 Z, V4 W) : x(X), y(Y), z(Z), w(W) {}

Matrix4::Matrix4(Matrix3 m) : x(m.x, 0), y(m.y, 0), z(m.z, 0), w(0, 0, 0, 1) {}

const Matrix4 Matrix4::ID = Matrix4(
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
);

V4 Matrix4::Col(int i) {
	switch(i) {
		case 0: return V4(xx, yx, zx, wx); break;
		case 1: return V4(xy, yy, zy, wy); break;
		case 2: return V4(xz, yz, zz, wz); break;
		case 3: return V4(xw, yw, zw, ww); break;
	};
	Log::Fatal("Invalid column index for Matrix4");
	return V4(0, 0, 0, 0);
}

Matrix3 Matrix4::Submatrix() {
	return Matrix3(
		xx, xy, xz,
		yx, yy, yz,
		zx, zy, zz
	);
}

Matrix4 Matrix4::Transpose() {
	return Matrix4(Col(0), Col(1), Col(2), Col(3));
}

Matrix4 operator*(Matrix4 lhs, Matrix4 rhs) {
	return Matrix4(
		Dot(lhs.x, rhs.Col(0)),	Dot(lhs.x, rhs.Col(1)),	Dot(lhs.x, rhs.Col(2)),	Dot(lhs.x, rhs.Col(3)),
		Dot(lhs.y, rhs.Col(0)),	Dot(lhs.y, rhs.Col(1)),	Dot(lhs.y, rhs.Col(2)),	Dot(lhs.y, rhs.Col(3)),
		Dot(lhs.z, rhs.Col(0)),	Dot(lhs.z, rhs.Col(1)),	Dot(lhs.z, rhs.Col(2)),	Dot(lhs.z, rhs.Col(3)),
		Dot(lhs.w, rhs.Col(0)),	Dot(lhs.w, rhs.Col(1)),	Dot(lhs.w, rhs.Col(2)),	Dot(lhs.w, rhs.Col(3))
	);
}

V4 operator*(Matrix4 lhs, V4 rhs) {
	return V4(Dot(lhs.x, rhs), Dot(lhs.y, rhs), Dot(lhs.z, rhs), Dot(lhs.w, rhs));
}

V4 operator*(V4 lhs, Matrix4 rhs) {
	return V4(Dot(rhs.Col(0), lhs), Dot(rhs.Col(1), lhs), Dot(rhs.Col(2), lhs), Dot(rhs.Col(3), lhs));
}

bool operator==(Matrix4 lhs, Matrix4 rhs) {
	for (int i = 0; i < 16; i++) {
		if (fabs(lhs.element[i] - rhs.element[i]) > EPSILON) return false;
	}
	return true;
}

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| Complex numbers                                                                                                                        |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

Complex::Complex(float R, float I = 0) : r(R), i(I) {}

Complex& Complex::operator+=(Complex rhs) {
	r += rhs.r;
	i += rhs.i;
	return *this;
}

Complex& Complex::operator-=(Complex rhs) {
	r -= rhs.r;
	i -= rhs.i;
	return *this;
}

Complex& Complex::operator*=(Complex rhs) {
	Complex result = *this * rhs;
	r = result.r;
	i = result.i;
	return *this;
}

Complex& Complex::operator/=(Complex rhs) {
	Complex result = *this / rhs;
	r = result.r;
	i = result.i;
	return *this;
}

Complex operator+(Complex lhs, Complex rhs) {
	return Complex(lhs.r + rhs.r, lhs.i + rhs.i);
}

Complex operator-(Complex lhs, Complex rhs) {
	return Complex(lhs.r - rhs.r, lhs.i - rhs.i);
}

Complex operator*(Complex lhs, Complex rhs) {
	return Complex(lhs.r * rhs.r - lhs.i * rhs.i, lhs.r * rhs.i + lhs.i * rhs.r);
}

Complex operator/(Complex lhs, Complex rhs) {
	return lhs * rhs.Inverse();
}

Complex Complex::Conjugate() {
	return Complex(r, -i);
}

float Complex::Modulus() {
	return sqrt(r * r + i * i);
}

float Complex::Phase() {
	return atan2f(i, r);
}

Complex Complex::Inverse() {
	return Conjugate() / Modulus();
}

Complex Expi(float alpha) {
	return Complex(cos(alpha), sin(alpha));
}

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| Quaternions                                                                                                                            |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

Quaternion::Quaternion(float C = 1.0f, float I = 0.0f, float J = 0.0f, float K = 0.0f) : c(C), i(I), j(J), k(K) {}

Quaternion::Quaternion(float angle, V3 vector) {
	float sin_angle = sinf(angle / 2);
	vector = Normalize(vector);
	c = cosf(angle / 2.0);
	i = sin_angle * vector.x;
	j = sin_angle * vector.y;
	k = sin_angle * vector.z;
}

Quaternion Quaternion::Conjugate() {
	return Quaternion(c, -i, -j, -k);
}

Quaternion operator*(Quaternion lhs, Quaternion rhs) {
	return { 
		lhs.c * rhs.c - lhs.i * rhs.i - lhs.j * rhs.j - lhs.k * rhs.k,
		lhs.c * rhs.i + lhs.i * rhs.c + lhs.j * rhs.k - lhs.k * rhs.j,
		lhs.c * rhs.j + lhs.j * rhs.c + lhs.k * rhs.i - lhs.i * rhs.k,
		lhs.c * rhs.k + lhs.k * rhs.c + lhs.i * rhs.j - lhs.j * rhs.i,
	};
}

Quaternion operator+(Quaternion lhs, Quaternion rhs) {
	return {lhs.c + rhs.c, lhs.i + rhs.i, lhs.j + rhs.j, lhs.k + rhs.k };
}

Quaternion operator-(Quaternion lhs, Quaternion rhs) {
	return { lhs.c - rhs.c, lhs.i - rhs.i, lhs.j - rhs.j, lhs.k - rhs.k };
}

V3 operator*(Quaternion lhs, V3 rhs) {
	Quaternion v = Quaternion(0.0, rhs.x, rhs.y, rhs.z);
	Quaternion result = lhs.Conjugate() * v * lhs;
	return V3(result.i, result.j, result.k);
}

Basis operator*(Quaternion lhs, Basis rhs) {
	return Basis(
		lhs * rhs.x,
		lhs * rhs.y,
		lhs * rhs.z
	);
}

bool operator==(Quaternion lhs, Quaternion rhs) {
	return lhs.c == rhs.c && lhs.i == rhs.i && lhs.j == rhs.j && lhs.k == rhs.k; 
}

bool operator!=(Quaternion lhs, Quaternion rhs) {
	return lhs.c != rhs.c || lhs.i == rhs.i || lhs.j == rhs.j || lhs.k == rhs.k; 
}

Matrix3::Matrix3(Quaternion q) :
	xx(2.0f * (q.c * q.c + q.i * q.i) - 1.0f),
	xy(2.0f * (q.i * q.j - q.c * q.k)),
	xz(2.0f * (q.i * q.k + q.c * q.j)),
	yx(2.0f * (q.i * q.j + q.c * q.k)),
	yy(2.0f * (q.c * q.c + q.j * q.j) - 1.0f),
	yz(2.0f * (q.j * q.k - q.c * q.i)),
	zx(2.0f * (q.i * q.k - q.c * q.j)),
	zy(2.0f * (q.j * q.k + q.c * q.i)),
	zz(2.0f * (q.c * q.c + q.k * q.k) - 1.0f)
{}

/*
+----------------------------------------------------------------------------------------------------------------------------------------+
| Transform                                                                                                                              |
+----------------------------------------------------------------------------------------------------------------------------------------+
*/

Scale::Scale(float X = 1.0f, float Y = 1.0f, float Z = 1.0f) : x(X), y(Y), z(Z) {};

Scale operator*(float lhs, Scale rhs) {
	return Scale(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

Scale operator*(Scale lhs, Scale rhs) {
	return Scale(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

V3 operator*(Scale lhs, V3 rhs) {
	return V3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

Basis operator*(Scale lhs, Basis rhs) {
	return Basis(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

bool operator==(Scale lhs, Scale rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

bool operator!=(Scale lhs, Scale rhs) {
	return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z;
}

Transform::Transform(V3 t, Quaternion r, Scale s) : translation(t), rotation(r), scale(s) {}

const Transform Transform::ID = Transform(V3(0,0,0), Quaternion(), Scale());

V3 operator*(Transform lhs, V3 rhs) {
	return lhs.translation + lhs.rotation * (lhs.scale * rhs);
}

V4 operator*(Transform lhs, V4 rhs) {
	V3 v = lhs.rotation * (lhs.scale * V3(rhs.x, rhs.y, rhs.z)) + rhs.w * lhs.translation;
	return V4(v, rhs.w);
}

Basis operator*(Transform lhs, Basis rhs) {
	return lhs.rotation * (lhs.scale * rhs);
}

Transform operator*(Transform lhs, Transform rhs) {
	return Transform(
		rhs.translation + rhs.rotation * lhs.translation,
		lhs.rotation * rhs.rotation,
		lhs.scale * rhs.scale
	);
}

bool operator==(Transform lhs, Transform rhs) {
	return lhs.translation == rhs.translation && 
	       lhs.scale == rhs.scale && 
		   lhs.rotation == rhs.rotation;
}

bool operator!=(Transform lhs, Transform rhs) {
	return lhs.translation != rhs.translation || 
	       lhs.scale == rhs.scale || 
		   lhs.rotation == rhs.rotation;
}

Matrix4 TRSMatrix(Transform transform) {
	Matrix3 rotation = transform.rotation;
	Matrix4 result = Matrix4(
		transform.scale.x * V4(rotation.xx, rotation.xy, rotation.xz, 0),
		transform.scale.y * V4(rotation.yx, rotation.yy, rotation.yz, 0),
		transform.scale.z * V4(rotation.zx, rotation.zy, rotation.zz, 0),
		V4(transform.translation.x, transform.translation.y, transform.translation.z, 1.0f)
	);
	return result;
}

/*
+----------------------------------------------------------------------------------------------------------------------------------------------+
| Fast Fourier Transform                                                                                                                       |
+----------------------------------------------------------------------------------------------------------------------------------------------+
*/

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
void DFT(uint32 N, Complex* input, Complex* output) {
	for (int i = 0; i < N; i++) {
		Complex result = Complex(0,0);
		for (int j = 0; j < N; j++) {
			result += input[j]*Expi(-TAU*j*i/N);
		}
		output[i] = result;
	}
}

// Twiddle factors preparation
void PrepareTwiddleFactors(uint32 N, Complex* twiddle) {
	twiddle[0] = Complex(1.0f, 0.0f);
	twiddle[N >> 1] = Complex(-1.0f, 0.0f);
	twiddle[N >> 2] = Complex(0.0f, -1.0f);
	twiddle[3*(N >> 2)] = Complex(0.0f, 1.0f);

	for (int i = 0; i < N; i++) {
		if      (i == 0)          twiddle[i] = Complex(1.0f, 0.0f);
		else if (i == N >> 1)     twiddle[i] = Complex(-1.0f, 0.0f);
		else if (i == N >> 2)     twiddle[i] = Complex(0.0f, -1.0f);
		else if (i == 3*(N >> 2)) twiddle[i] = Complex(0.0f, 1.0f);
		else                      twiddle[i] = Expi(-TAU*i/N);
	}
}

void FFTButterfly(uint32 stride, Complex* data, Complex twiddle) {
	Complex x0 = data[0];
	Complex x1 = data[stride];

	data[0]      = x0 + twiddle*x1;
	data[stride] = x0 - twiddle*x1;
}

void FFT(uint32 N, Complex* input, Complex* output, Complex* twiddle) {
	uint32 log2N = log2(N);
	
	// Bit reverse input
	for (int i = 0; i < N; i++) {
		output[i] = input[BitReverse(i, log2N)];
	}

	// FFT stages
	uint32 stride = 1;
	uint32 twiddle_stride = N >> 1;
	for (int p = 0; p < log2N; p++) {		
		for (int block = 0; block < N; block += stride << 1)
		for (int k = 0; k < stride; k++) {
			FFTButterfly(stride, &output[block + k], twiddle[k * twiddle_stride]);
		}
		
		stride <<= 1;
		twiddle_stride >>= 1;
	}
}
