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
