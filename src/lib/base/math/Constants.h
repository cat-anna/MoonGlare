#pragma once

namespace emath::constant {

// pi
template<typename T> static constexpr T pi = 3.14159265358979323846;
// pi / 2 
template<typename T> static constexpr T pi_2 = pi<T> / T(2);
// pi / 4
template<typename T> static constexpr T pi_4 = pi<T> / T(4);
// 1 / pi
template<typename T> static constexpr T _1_pi = T(1) / pi<T>;
// 2 / pi
template<typename T> static constexpr T _2_pi = T(2) / pi<T>;

// e
template<typename T> static constexpr T e = 2.71828182845904523536;

template<typename T> static constexpr T epsilon = 1.0e-5;// SIMD_EPSILON;

//2 / sqrt(pi)	M_2_SQRTPI	1.12837916709551257390
//sqrt(2)	M_SQRT2	1.41421356237309504880
//1 / sqrt(2)	M_SQRT1_2	0.707106781186547524401
//log_2(e)	M_LOG2E	1.44269504088896340736
//log_10(e)	M_LOG10E	0.434294481903251827651
//log_e(2)	M_LN2	0.693147180559945309417
//log_e(10)	M_LN10	2.30258509299404568402

}
