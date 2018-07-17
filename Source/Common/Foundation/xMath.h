#pragma once

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
	T min(T a, T b) { return a < b ? a : b; }

	template<class T>
	T max(T a, T b) { return a < b ? b : a; }

	struct RawVec3 {
		union {
			struct { float r, g, b; };
			struct { float x, y, z; };
			float m_Raw[3];
		};

		operator const math::fvec3&() const { return reinterpret_cast<const math::fvec3&>(*this); }
		void operator = (const math::fvec3& other) { reinterpret_cast<math::fvec3&>(*this) = other; }
		void operator = (const emath::fvec3& other) { reinterpret_cast<math::fvec3&>(*this) = emath::MathCast<math::fvec3>(other); }

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
		void operator = (const emath::fvec4& other) { reinterpret_cast<math::fvec4&>(*this) = emath::MathCast<math::fvec4>(other); }
							   
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
		void operator = (const emath::fmat4& other) { reinterpret_cast<math::mat4&>(*this) = emath::MathCast<math::mat4>(other); }

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
