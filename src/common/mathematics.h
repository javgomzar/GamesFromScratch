#pragma once

#include <platform.h>
#include "enum.h"
#include <tokenizer.h>


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
    static bool Bernoulli(float p = 0.5f);
    static int RandInt(int min, int max);
    static float Normal();
    static float Normal(float Mean, float StdDeviation);
};

#define RandomEnum(Type) (Type)_RandomEnum(debug_##Type)