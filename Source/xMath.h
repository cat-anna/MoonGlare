/*
 * d3math.h
 *
 *  Created on: 28-11-2013
 *      Author: Paweu
 */

#ifndef XMATH_H
#define XMATH_H
 
namespace Physics {
	using vec3 = btVector3;
	using Quaternion = btQuaternion;
	using Transform = btTransform;
}

namespace math {
	typedef glm::fvec2 vec2;
	typedef glm::fvec3 vec3;
	typedef glm::fvec4 vec4;

	typedef glm::fvec2 fvec2;
	typedef glm::fvec3 fvec3;
	typedef glm::fvec4 fvec4;

	typedef glm::fmat4 mat4;

	typedef glm::uvec2 uvec2;
	typedef glm::uvec3 uvec3;
	typedef glm::uvec4 uvec4;

	using Transform = Physics::Transform;
	using Quaternion = Physics::Quaternion;

	template<class T>
	inline T clamp(T t, T tmin, T tmax) {
		if (t > tmax) return tmax;
		if (t < tmin) return tmin;
		return t;
	}
 
	template<class T>
	T min(T a, T b) { return a < b ? a : b; }

	template<class T>
	T max(T a, T b) { return a < b ? b : a; }

	template<class VEC, class POS>
	inline VEC LinearVectorMix(const VEC &v1, const VEC &v2, POS t) {
		return v1 * t + v2 * (static_cast<POS>(1.0) - t);
	}

	/** Calculate plane normal from 3 points */
	template<class VEC> inline VEC PlaneNormal(const VEC &p1, const VEC &p2, const VEC &p3) {
		return PlaneNormalVectors(p2 - p1, p3 - p1);
	}

	/** Calculate plane normal from 2 points and vector */
	template<class VEC> inline VEC PlaneNormalPointPointVector(const VEC &p1, const VEC &p2, const VEC &v1) {
		return PlaneNormalVectors(p2 - p1, v1);
	}

	/** Calculate plane normal from 2 vectors */
	inline Physics::vec3 PlaneNormalVectors(const Physics::vec3 &v1, const Physics::vec3 &v2) {
		return v1.cross(v2);
	}

	template<class T>
	inline T next_power2(T in) {
		T r = 1;
		if (std::is_integral<T>::value)
			while (r < in) r <<= 1;
		else
			while (r < in) r *= static_cast<T>(2.0);
		return r;
	}

	struct RawVec3 {
		union {
			struct { float r, g, b; };
			struct { float x, y, z; };
			float m_Raw[3];
		};

		operator const math::fvec3&() const { return reinterpret_cast<const math::fvec3&>(*this); }
		void operator = (const math::fvec3& other) { reinterpret_cast<math::fvec3&>(*this) = other; }

		float& operator[](size_t idx) { return m_Raw[idx]; };
		const float& operator[](size_t idx) const { return m_Raw[idx]; };

		float Max() const { return fmax(fmax(r, g), b); }
	};
	static_assert(std::is_pod<RawVec3>::value, "RawVec3 shall be POD!");
	static_assert(sizeof(RawVec3) == sizeof(math::fvec3), "RawVec3 and fvec3 size mismatch!");

	struct RawVec4 {
		union {
			struct { float r, g, b, a; };
			struct { float x, y, z, w; };
			float m_Raw[4];
		};

		operator const math::fvec4&() const { return reinterpret_cast<const math::fvec4&>(*this); }
		void operator = (const math::fvec4& other) { reinterpret_cast<math::fvec4&>(*this) = other; }

		float& operator[](size_t idx) { return m_Raw[idx]; };
		const float& operator[](size_t idx) const { return m_Raw[idx]; };

		float Max() const { return fmax(fmax(r, g), fmax(b, a)); }
	};
	static_assert(std::is_pod<RawVec4>::value, "RawVec4 shall be POD!");
	static_assert(sizeof(RawVec4) == sizeof(math::fvec4), "RawVec4 and fvec3 size mismatch!");

	struct RawMat4 {
		RawVec4 m_Mat[4];

		operator const math::mat4&() const { return reinterpret_cast<const math::mat4&>(*this); }
		void operator = (const math::mat4& other) { reinterpret_cast<math::mat4&>(*this) = other; }

		RawVec4& operator[](size_t idx) { return m_Mat[idx]; };
		const RawVec4& operator[](size_t idx) const { return m_Mat[idx]; };
	};
	static_assert(std::is_pod<RawMat4>::value, "RawMat4 shall be POD!");
	static_assert(sizeof(RawMat4) == sizeof(math::mat4), "RawMat4 and mat4 size mismatch!");

	using RGB = RawVec3;
	using RGBS = RawVec4;
}

inline Physics::vec3 convert(const math::vec3& src) { return Physics::vec3(src[0], src[1], src[2]); }
inline math::vec3 convert(const Physics::vec3& src) { return math::vec3(src[0], src[1], src[2]); }

inline math::Quaternion convert(const math::vec4& src) { return math::Quaternion(src[0], src[1], src[2], src[3]); }
inline math::vec4 convert(const math::Quaternion& src) { return math::vec4(src[0], src[1], src[2], src[3]); }

using math::vec2;
using math::vec4;
using math::uvec3;
using math::uvec2;

using math::max;
using math::min;

#endif // XMATH_H 
