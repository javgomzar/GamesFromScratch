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

// +----------------------------------------------------------------------------------------------------------------------------------------+
// | RNG                                                                                                                                    |
// +----------------------------------------------------------------------------------------------------------------------------------------+

void RNG::NextState() {
	state ^= state << 13;
	state ^= state >> 7;
	state ^= state << 17;
}

uint64 RNG::Seed() {
#ifdef _DEBUG
    time_t seconds = time(nullptr);
    tm* local_time = localtime(&seconds);
    seed = local_time->tm_mday + 123456789;
#else
    seed = Platform::GetWallClock();
#endif
    for (int i = 0; i < 8; i++) {
        RNG::NextState();
    }

    return seed;
}

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

int RNG::_RandomEnum(Type type) {
    int i = ENUM_VALUES_SIZE;
    while(i > 0) {
        Debug_Enum_Value entry = ENUM_VALUES[--i];
        if (entry.enum_type == type) {
            return RandInt(0, entry.value);
        }
    }
	// Type wasn't found in enum list.
    assert(false);
    return 0;
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
		default: Log::Fatal("Invalid column index for Matrix2.");
	};
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
		case 0: return V3(xx, yx, zx); break;
		case 1: return V3(xy, yy, zy); break;
		case 2: return V3(xz, yz, zz); break;
		default: Log::Fatal("Invalid column index for Matrix3.");
	};
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
	V3 y = cross(x, V3(1,0,0));
	if (modulus(y) < EPSILON) {
		y = cross(x, V3(0,1,0));
		if (modulus(y) < EPSILON) {
			y = cross(x, V3(0,0,1));
		}
	}
	return Complete(x, normalize(y));
}

Basis normalize(Basis basis) {
	Basis result;
	result.x = normalize(basis.x);
	result.y = normalize(basis.y);
	result.z = normalize(basis.z);
	return result;
}

const Matrix4 Matrix4::ID = Matrix4(
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
);

V4 col(Matrix4 A, int i) {
	switch(i) {
		case 0: return V4(A.xx, A.yx, A.zx, A.wx); break;
		case 1: return V4(A.xy, A.yy, A.zy, A.wy); break;
		case 2: return V4(A.xz, A.yz, A.zz, A.wz); break;
		case 3: return V4(A.xw, A.yw, A.zw, A.ww); break;
		default: Log::Fatal("Invalid column index for Matrix4");
	};
}

Matrix4 transpose(Matrix4 A) {
	Matrix4 result;
	result.x = col(A, 0);
	result.y = col(A, 1);
	result.z = col(A, 2);
	result.w = col(A, 3);
	return result;
}

Matrix4 operator*(Matrix4 A, Matrix4 B) {
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

V4 operator*(Matrix4 A, V4 V) {
	V4 result;
	result.x = dot(A.x, V);
	result.y = dot(A.y, V);
	result.z = dot(A.z, V);
	result.w = dot(A.w, V);
	return result;
}

V4 operator*(V4 V, Matrix4 A) {
	V4 result;
	result.x = dot(col(A, 0), V);
	result.y = dot(col(A, 1), V);
	result.z = dot(col(A, 2), V);
	result.w = dot(col(A, 3), V);
	return result;
}

bool operator==(Matrix4 A, Matrix4 B) {
	for (int i = 0; i < 16; i++) {
		if (fabs(A.element[i] - B.element[i]) > EPSILON) return false;
	}
	return true;
}

Matrix3 Submatrix3(Matrix4 Matrix) {
	return {
		Matrix.xx, Matrix.xy, Matrix.xz,
		Matrix.yx, Matrix.yy, Matrix.yz,
		Matrix.zx, Matrix.zy, Matrix.zz
	};
}

Matrix4 Matrix4FromMatrix3(Matrix3 m) {
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

complex conjugate(complex A) {
	return {
		A.r,
		-A.i
	};
}

complex operator+(complex A, float B) {
	return {
		A.r + B,
		A.i
	};
}

complex& operator+=(complex& A, float B) {
	A.r += B;
	return A;
}

complex operator+(float A, complex B) {
	return {
		A + B.r,
		B.i
	};
}

complex operator-(float A, complex B) {
	return {
		A - B.r,
		B.i
	};
}

complex operator-(complex A, float B) {
	return {
		A.r - B,
		A.i
	};
}

complex& operator-=(complex& A, float B) {
	A.r -= B;
	return A;
}

complex operator*(complex A, float C) {
	return {
		A.r * C,
		A.i * C
	};
}

complex operator*(float C, complex A) {
	return {
		A.r * C,
		A.i * C
	};
}

complex& operator*=(complex& A, float B) {
	A.r *= B;
	A.i *= B;
	return A;
}

complex operator/(complex A, float C) {
	return {
		A.r / C,
		A.i / C
	};
}

complex& operator/=(complex& A, float B) {
	A.r /= B;
	A.i /= B;
	return A;
}

float modulus(complex A) {
	return sqrt(A.r * A.r + A.i * A.i);
}

float phase(complex A) {
	return atan2f(A.i, A.r);
}

complex inverse(complex A) {
	return conjugate(A) / (A.r * A.r + A.i * A.i);
}

complex expi(float Alpha) {
	return {
		cos(Alpha),
		sin(Alpha)
	};
}

complex operator+(complex A, complex B) {
	return {
		A.r + B.r,
		A.i + B.i
	};
}

complex& operator+=(complex& A, complex B) {
	A.r += B.r;
	A.i += B.i;
	return A;
}

complex operator-(complex A, complex B) {
	return {
		A.r - B.r,
		A.i - B.i
	};
}

complex& operator-=(complex& A, complex B) {
	A.r -= B.r;
	A.i -= B.i;
	return A;
}

complex operator*(complex A, complex B) {
	return {
		A.r * B.r - A.i * B.i,
		A.r * B.i + A.i * B.r
	};
}

complex& operator*=(complex& A, complex B) {
	A = A * B;
	return A;
}

complex operator/(complex A, complex B) {
	return A * inverse(B);
}

complex operator/(float A, complex B) {
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

Quaternion BuildQuaternion(float angle, V3 Vector) {
	float sin_angle = sinf(angle / 2);
	Vector = normalize(Vector);
	return { cosf(angle / 2.0), sin_angle * Vector.x, sin_angle * Vector.y, sin_angle * Vector.z};
}

Quaternion Conjugate(Quaternion q) {
	return { q.c, -q.i, -q.j, -q.k};
}

Quaternion operator*(Quaternion q1, Quaternion q2) {
	return { 
		q1.c * q2.c - q1.i * q2.i - q1.j * q2.j - q1.k * q2.k,
		q1.c * q2.i + q1.i * q2.c + q1.j * q2.k - q1.k * q2.j,
		q1.c * q2.j + q1.j * q2.c + q1.k * q2.i - q1.i * q2.k,
		q1.c * q2.k + q1.k * q2.c + q1.i * q2.j - q1.j * q2.i,
	};
}

Quaternion operator*(float c, Quaternion q) {
	return { c * q.c, c * q.i, c * q.j, c * q.k};
}

Quaternion operator*(Quaternion q, float c) {
	return { c * q.c, c * q.i, c * q.j, c * q.k };
}

Quaternion operator+(Quaternion q1, Quaternion q2) {
	return {q1.c + q2.c, q1.i + q2.i, q1.j + q2.j, q1.k + q2.k };
}

Quaternion operator-(Quaternion q1, Quaternion q2) {
	return { q1.c - q2.c, q1.i - q2.i, q1.j - q2.j, q1.k - q2.k };
}

Quaternion operator-(Quaternion q) {
	return { -q.c, -q.i, -q.j, -q.k };
}

V3 operator*(Quaternion Q, V3 Vector) {
	Quaternion Q_ = Conjugate(Q);
	Quaternion V = Quaternion(0.0, Vector.x, Vector.y, Vector.z);
	Quaternion result = Q_ * V * Q;
	return V3(result.i, result.j, result.k);
}

Basis operator*(Quaternion Q, Basis basis) {
	Basis result;
	result.x = Q * basis.x;
	result.y = Q * basis.y;
	result.z = Q * basis.z;

	return result;
}

bool operator==(Quaternion Q1, Quaternion Q2) {
	return Q1.c == Q2.c && Q1.i == Q2.i && Q1.j == Q2.j && Q1.k == Q2.k; 
}

bool operator!=(Quaternion Q1, Quaternion Q2) {
	return Q1.c != Q2.c || Q1.i == Q2.i || Q1.j == Q2.j || Q1.k == Q2.k; 
}

Matrix3 Rotation_Matrix(Quaternion Q) {
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

Scale BuildScale(float x = 1.0, float y = 1.0, float z = 1.0) {
	return { x, y, z };
};

Scale operator*(float C, Scale S) {
	return { C * S.x, C * S.y, C * S.z };
}

Scale operator*(Scale S, Scale T) {
	return BuildScale(S.x * T.x, S.y * T.y, S.z * T.z);
}

V3 operator*(Scale S, V3 Vector) {
	return {S.x * Vector.x, S.y * Vector.y, S.z * Vector.z};
}

Basis operator*(Scale S, Basis basis) {
	Basis result;
	result.x = S.x * basis.x;
	result.y = S.y * basis.y;
	result.z = S.z * basis.z;
	return result;
}

bool operator==(Scale scale1, Scale scale2) {
	return scale1.x == scale2.x && scale1.y == scale2.y && scale1.z == scale2.z;
}

bool operator!=(Scale scale1, Scale scale2) {
	return scale1.x != scale2.x || scale1.y != scale2.y || scale1.z != scale2.z;
}

INTROSPECT
struct Transform {
	V3 translation;
	Scale scale;
	Quaternion rotation;
};

Transform IdentityTransform = { V3(0,0,0), BuildScale(), Quaternion(1.0f) };

Transform GetTransform(Quaternion Rotation, V3 Translation = V3(0,0,0), Scale Scaling = BuildScale()) {
	return { Translation, Scaling, Rotation };
}

Transform GetTransform(V3 Translation, Quaternion Rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f), Scale Scaling = BuildScale()) {
	return { Translation, Scaling, Rotation };
}

Transform GetTransform(V3 Translation, Scale Scaling, Quaternion Rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f)) {
	return { Translation, Scaling, Rotation };
}

V3 operator*(Transform transform, V3 Vector) {
	return transform.rotation * (transform.scale * Vector) + transform.translation;
}

V4 operator*(Transform transform, V4 vector) {
	V3 v = transform.rotation * (transform.scale * V3(vector.x, vector.y, vector.z)) + vector.w * transform.translation;
	return { v.x, v.y, v.z, vector.w};
}

Basis operator*(Transform transform, Basis basis) {
	return transform.rotation * basis;
}

Transform operator*(Transform transform1, Transform transform2) {
	Transform result = { 0 };
	result.scale = transform1.scale * transform1.scale;
	result.translation = transform2.translation + transform2.rotation * transform2.translation;
	result.rotation = transform1.rotation * transform2.rotation;
	return result;
}

bool operator==(Transform transform1, Transform transform2) {
	return transform1.translation == transform2.translation && 
	       transform1.scale == transform2.scale && 
		   transform1.rotation == transform2.rotation;
}

bool operator!=(Transform transform1, Transform transform2) {
	return transform1.translation != transform2.translation || 
	       transform1.scale == transform2.scale || 
		   transform1.rotation == transform2.rotation;
}

Matrix4 TRS_Matrix(Transform transform) {
	Matrix3 rotation = Rotation_Matrix(transform.rotation);
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

V3 ClosestPoint(line3 Line, V3 Point) {
	float t = dot(Line.Direction, Point - Line.Point);
	return Point + t * Line.Direction;
}

float Distance(line3 Line, V3 Point) {
	return dot(Line.Direction, Point - Line.Point);
}

float Distance(line3 Line1, line3 Line2) {
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

Ray BuildRay(V3 point, V3 direction) {
	return { point, normalize(direction) };
}

V3 ClosestPoint(Ray ray, V3 point) {
	float t = dot(ray.direction, point - ray.point);
	if (t <= 0.0f) return ray.point;
	else return ray.point + t * ray.direction;
}

float SqDistance(Ray ray, V3 point) {
	V3 D = point - ClosestPoint(ray, point);
	return dot(D,D);
}

struct Segment2 {
	V2 head;
	V2 tail;
};

bool AreParallel(Segment2 S1, Segment2 S2) {
	return AreParallel(S1.tail - S1.head, S2.tail - S2.head);
}

bool IsInside(Segment2 S, V2 P) {
	V2 u = S.head - P;
	V2 v = S.tail - P;

	return AreParallel(u,v) && dot(u,v) <= 0;
}

/*
	Intersects two 2D segments. If result is 0, there is no intersection. If result is 1, the intersection is exactly one point,
	which will be given in the `IntersectionPoint` parameter. If result is 2, the two segments overlap over another segment, which will
	be given in the `IntersectionSegment` parameter.
*/
int Intersect(Segment2 S1, Segment2 S2, V2* IntersectionPoint, Segment2* IntersectionSegment) {
	if (AreParallel(S1, S2)) {
		V2 u = S1.tail - S1.head;
		V2 v = S2.head - S1.head;
		if (AreParallel(u, v)) {
			float R0 = dot(u, u);                 // Coordinates for S1.Tail
			float R1 = dot(u, v);                 // Coordinates for S2.Head
			float R2 = dot(u, S2.tail - S1.head); // Coordinates for S2.Tail

			// Swap if segments point in opposite directions
			bool Swap = R2 - R1 < 0;
			if (Swap) {
				float Temp = R1;
				R1 = R2;
				R2 = Temp;
				V2 Temp2 = S2.head;
				S2.head = S2.tail;
				S2.tail = Temp2;
			}

			if (fabsf(R1) < EPSILON) {
				if (R2 < EPSILON) {
					*IntersectionPoint = S2.head;
					return 1;
				}
				else {
					*IntersectionSegment = S2;
					if (R2 > R0) {
						IntersectionSegment->tail = S1.tail;
					}
					return 2;
				}
			}
			else if (R1 < 0) {
				if (fabsf(R2) < EPSILON) {
					*IntersectionPoint = S2.tail;
					return 1;
				}
				else if (R2 < 0) {
					return 0;
				}
				else {
					*IntersectionSegment = S1;
					if (R2 < R0) {
						IntersectionSegment->tail = S2.tail;
					}
					return 2;
				}
			}
			else {
				if (fabsf(R1 - R0) < EPSILON) {
					*IntersectionPoint = S2.head;
					return 1;
				}
				else if (R1 > R0) {
					return 0;
				}
				else {
					IntersectionSegment->head = S2.head;
					IntersectionSegment->tail = S1.tail;
					return 2;
				}
			}
		}
		return 0;
	}
	else {
		V2 A = S1.head;
		V2 B = S1.tail;
		V2 C = S2.head;
		V2 D = S2.tail;

		float Orient1 = cross(B - A, C - A);
		float Orient2 = cross(B - A, D - A);
		float Orient3 = cross(D - C, A - C);
		float Orient4 = cross(D - C, B - C);

		if (Orient1 * Orient2 > 0 || Orient3 * Orient4 > 0) {
			return 0;
		}
		else {
			float t = - Orient1 / cross(B - A, D - C);
			assert(t >= 0 && t <= 1);
			*IntersectionPoint = C + t * (D - C);
			return 1;
		}
	}
	return 0;
}

V2 ClosestPoint(Segment2 Segment, V2 Point) {
	V2 V = Segment.tail - Segment.head;

	float t = dot(V, Point);
	if (t <= 0.0f) return Segment.head;
	float Denom = dot(V, V);
	if (t >= Denom) return Segment.tail;
	else return Segment.head + (t / Denom) * V;
}

float SqDistance(Segment2 Segment, V2 Point) {
	V2 V = Segment.tail - Segment.head;
	V2 R = Point - Segment.head;

	R = project(R, V) - Point;
	return dot(R,R);
}

float SqDistance(Segment2 Segment1, Segment2 Segment2) {
	float d1 = SqDistance(Segment1, Segment2.head);
	float d2 = SqDistance(Segment1, Segment2.tail);
	float d3 = SqDistance(Segment2, Segment1.head);
	float d4 = SqDistance(Segment2, Segment2.tail);

	return fmin(d1, fmin(d2, fmin(d3, d4)));
}

INTROSPECT
struct Segment3 {
	V3 head;
	V3 tail;
};

Segment3 operator*(Transform transform, Segment3 segment) {
	return { transform * segment.head, transform * segment.tail };
}

/*
	Returns the transform that moves the segment [origin, (0,L,0)] to the input segment, L being the length of this segment.
*/
Transform SegmentTransform(Segment3 segment) {
	Transform result;
	result.scale = BuildScale();
	result.translation = segment.head;
	V3 V = segment.tail - segment.head;
	float L = modulus(V);
	float angle = acosf(V.y / L);
	result.rotation = BuildQuaternion(angle, V3(-V.z,0,V.x));
	return result;
}

V3 ClosestPoint(Segment3 segment, V3 point) {
	V3 V = segment.tail - segment.head;

	float t = dot(V, point);
	if (t <= 0.0f) return segment.head;
	float denom = dot(V, V);
	if (t >= denom) return segment.tail;
	else return segment.head + (t / denom) * V;
}

float SqDistance(Segment3 segment, V3 point) {
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

float SqDistance(Segment3 segment1, Segment3 segment2) {
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

float SqDistance(Segment3 segment, Ray ray) {
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

float GetArea(triangle2 T) {
	return 0.5f * cross(T.Points[2] - T.Points[0], T.Points[1] - T.Points[0]);
}

bool IsInside(triangle2 T, V2 P) {
	triangle2 U = { P, T.Points[1], T.Points[2] };
	triangle2 V = { P, T.Points[2], T.Points[0] };
	triangle2 W = { P, T.Points[0], T.Points[1] };

	float Area0 = GetArea(U);
	float Area1 = GetArea(V);
	float Area2 = GetArea(W);

	if (Area0 * Area1 > 0 && Area1 * Area2 > 0) {
		return true;
	}

	Segment2 S1 = { T.Points[0], T.Points[1] };
	Segment2 S2 = { T.Points[1], T.Points[2] };
	Segment2 S3 = { T.Points[2], T.Points[0] };

	return IsInside(S1, P) || IsInside(S2, P) || IsInside(S3, P);
}

float SqDistance(triangle2 T, V2 P) {
	Segment2 E[3] = {
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

struct Rect {
	float left;
	float top;
	float width;
	float height;
};

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
	V2 C1 = Center(rect1);
	V2 C2 = Center(rect2);
	return fabsf(C1.x - C2.x) < 0.5f * (rect1.width  + rect2.width) && 
	       fabsf(C1.y - C2.y) < 0.5f * (rect1.height + rect2.height);
}

struct vector_plane {
	V3 Normal;
};

vector_plane VectorPlane(V3 normal) {
	return { normalize(normal) };
}

struct affine_plane {
	float c; // The equation of the plane is dot(Normal, x) = c
	V3 Normal;
};

affine_plane AffinePlane(V3 Point, V3 Normal) {
	Normal = normalize(Normal);
	return { dot(Normal, Point), Normal };
}

V3 ClosestPoint(affine_plane Plane, V3 Point) {
	return Point - (dot(Point, Plane.Normal) - Plane.c) * Plane.Normal;
}

float Distance(affine_plane Plane, V3 Point) {
	return dot(Point, Plane.Normal) - Plane.c;
}

// +----------------------------------------------------------------------------------------------------------------------------------------------+
// | Colliders                                                                                                                                    |
// +----------------------------------------------------------------------------------------------------------------------------------------------+

enum Collider_Type {
    Rect_Collider,
    Cube_Collider,
    Sphere_Collider,
	Capsule_Collider
};

struct Collider {
    Collider_Type type;
    V3 offset;
    union {
		struct {
			float half_width;
			float half_height;
		} rect;
		struct {
			float half_width;
			float half_height;
			float half_depth;
		} cube;
		struct {
			float radius;
		} sphere;
		struct {
			Segment3 segment;
			float distance;
		} capsule;
	};
};

Collider RectCollider(V2 offset, float width, float height) {
	Collider result = {};
	result.type = Rect_Collider;
	result.offset = V3(offset.x, offset.y, 0);
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
	Collider result = {};
	result.type = Cube_Collider;
	result.offset = offset;
	result.cube.half_width = 0.5f * width;
	result.cube.half_height = 0.5f * height;
	result.cube.half_depth = 0.5f * depth;
	return result;
}

Collider SphereCollider(V3 offset, float radius) {
	Collider result = {};
	result.type = Sphere_Collider;
	result.offset = offset;
	result.sphere.radius = radius;
	return result;
}

Collider CapsuleCollider(V3 head, V3 tail, float distance) {
	Collider result = {};
	result.type = Capsule_Collider;
	result.offset = 0.5f * (head + tail);
	result.capsule.segment = {head, tail};
	result.capsule.distance = distance;
	return result;
}

Collider operator*(Transform T, Collider C) {
	switch(C.type) {
        case Rect_Collider: {
            V3 Displacement = T.translation;
			Displacement.z = 0;
			C.offset += Displacement;
			return C;
        } break;

		case Sphere_Collider:
        case Cube_Collider: {
            V3 Displacement = T.translation;
			C.offset += Displacement;
			return C;
        } break;

		case Capsule_Collider: {
			C.capsule.segment = T * C.capsule.segment;
			return C;
		} break;

		default: Log::Fatal("Invalid collider type.");
	}
	return C;
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
            return dot(position - collider.offset, position - collider.offset) <= collider.sphere.radius * collider.sphere.radius;
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
				float SumRadii = collider1.sphere.radius + collider2.sphere.radius;
				V3 r = collider2.offset - collider1.offset;
				return dot(r, r) <= SumRadii * SumRadii;
			} break;
			case Capsule_Collider: {
				float SumDistance = collider1.capsule.distance + collider1.capsule.distance;
				return SqDistance(collider1.capsule.segment, collider2.capsule.segment) <= SumDistance * SumDistance;
			} break;
			default: Log::Fatal("Invalid collider type");
		}
	}
	else {
		if (collider1.type > collider2.type) {
			Collider C = collider2;
			collider2 = collider1;
			collider1 = C;
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
				float SumDistances = collider1.sphere.radius + collider2.capsule.distance;
				return SqDistance(collider2.capsule.segment, collider1.offset) <= SumDistances * SumDistances;
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
