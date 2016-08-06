/*
  * Generated by cppsrc.sh
  * On 2015-07-09 17:53:39,50
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include <pch.h>
#include <MoonGlare.h>
#include <cmath>
#include "Math.h"

namespace MoonGlare {
namespace Core {
namespace Scripts {

int randomi(int rmin, int rmax){
	int d = rmax - rmin + 1;
	int r = (rand() % d) + rmin;
	// AddLog("random: " << rmin << "   " << r << "    " << rmax);
	return r;
}

int RandRange(int rmin, int rmax){
	int d = rmax - rmin + 1;
	int r = (rand() % d) + rmin;
	// AddLog("random: " << rmin << "   " << r << "    " << rmax);
	return r;
}

float randomf(float rmin, float rmax){
	float d = rmax - rmin;
	float r = (rand() % static_cast<int>(d)) + rmin;
	// AddLog("random: " << rmin << "   " << r << "    " << rmax);
	return r;
}

template<class T> T VecNormalized(const T * vec) { return glm::normalize(*vec); }
template<class T> void VecNormalize(T *vec) { *vec = glm::normalize(*vec); }

template<class T> float VecDotProduct(T *a, T* b) { return glm::dot(*a, *b); }
template<class T> T VecAdd(T *a, T* b) { return *a + *b; }
template<class T> T VecSub(T *a, T* b) { return *a - *b; }
math::vec3 VecCrossProduct(math::vec3 *a, math::vec3* b) { return glm::cross(*a, *b); }
math::vec4 VecCrossProduct(math::vec4 *a, math::vec4* b) {
	auto &q1 = *a;
	auto &q2 = *b;
	math::vec4 out;
	out.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y;
	out.y = q1.w*q2.y + q1.y*q2.w + q1.z*q2.x - q1.x*q2.z;
	out.z = q1.w*q2.z + q1.z*q2.w + q1.x*q2.y - q1.y*q2.x;
	out.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
	return out;
}

template<class T>
std::string ToString(T *vec) { 
	std::ostringstream oss;
	oss << *vec;
	return oss.str();
}
inline std::string Vec2ToString(math::vec2 *vec) {
	auto &t = *vec;
	char b[256];
	sprintf(b, "vec2(%f, %f)", t[0], t[1]);
	return b;
}
inline std::string Vec3ToString(math::vec3 *vec) {
	auto &t = *vec;
	char b[256];
	sprintf(b, "vec3(%f, %f, %f)", t[0], t[1], t[2]);
	return b;
}
inline std::string Vec4ToString(math::vec4 *vec) {
	auto &t = *vec;
	char b[256];
	sprintf(b, "vec3(%f, %f, %f, %f)", t[0], t[1], t[2], t[3]);
	return b;
}

void ScriptMath(ApiInitializer &root){
	struct T {
		static math::vec4 MakeVec4(float x, float y, float z, float w) {
			return math::vec4(x, y, z, w);
		}
		static math::vec3 MakeVec3(float x, float y, float z) {
			return math::vec3(x, y, z);
		}
		static math::vec2 MakeVec2(float x, float y) {
			return math::vec2(x, y);
		}

		static math::vec4 AxisAngle(float x, float y, float z, float a) {
			// Here we calculate the sin( theta / 2) once for optimization
			a = glm::radians(a);
			float factor = sin(a / 2.0f);

			// Calculate the x, y and z of the quaternion
			x *= factor;
			y *= factor;
			z *= factor;

			// Calcualte the w value by cos( theta / 2 )
			float w = cos(a / 2.0f);

			return glm::normalize(math::vec4(x, y, z, w));
		}
	};
	root
	//.addFunction("randomseed", &srand)
	//.addFunction("rand", &rand)
	.addFunction("random", &randomi)
	.addFunction("RandRange", &randomi)
	//.addFunction("randomf", &randomf)
	.addFunction("MakeVec4", &T::MakeVec4)
	.addFunction("MakeVec3", &T::MakeVec3)
	.addFunction("MakeVec2", &T::MakeVec2)
	.addFunction("QuaternionFromAxisAngle", &T::AxisAngle)

	.beginClass<math::vec4>("vec4")
		.addConstructor<void(*)(float, float, float, float)>()
		.addData("x", &math::vec4::x)
		.addData("y", &math::vec4::y)
		.addData("z", &math::vec4::z)
		.addData("w", &math::vec4::w)
		.addProperty<math::vec4, math::vec4>("normalized", &VecNormalized<math::vec4>)
		.addFunction("length", &math::vec4::length)
		.addFunction("normalize", Utils::Template::InstancedStaticCall<math::vec4, void>::callee<VecNormalize>())
		.addFunction("__tostring", Utils::Template::InstancedStaticCall<math::vec4, std::string>::callee<Vec4ToString>())

		.addFunction("__mul", Utils::Template::InstancedStaticCall<math::vec4, float, math::vec4*>::callee<VecDotProduct>())
		.addFunction("__add", Utils::Template::InstancedStaticCall<math::vec4, math::vec4, math::vec4*>::callee<VecAdd>())
		.addFunction("__sub", Utils::Template::InstancedStaticCall<math::vec4, math::vec4, math::vec4*>::callee<VecSub>())
		.addFunction("__mod", Utils::Template::InstancedStaticCall<math::vec4, math::vec4, math::vec4*>::callee<VecCrossProduct>())
	.endClass()

	.beginClass<math::vec3>("vec3")
		.addConstructor<void(*)(float, float, float)>()
		.addData("x", &math::vec3::x)
		.addData("y", &math::vec3::y)
		.addData("z", &math::vec3::z)
		.addProperty<math::vec3, math::vec3>("normalized", &VecNormalized<math::vec3>)
		.addFunction("length", &math::vec3::length)
		.addFunction("normalize", Utils::Template::InstancedStaticCall<math::vec3, void>::callee<VecNormalize>())
		.addFunction("__tostring", Utils::Template::InstancedStaticCall<math::vec3, std::string>::callee<Vec3ToString>())

		.addFunction("__mul", Utils::Template::InstancedStaticCall<math::vec3, float, math::vec3*>::callee<VecDotProduct>())
		.addFunction("__add", Utils::Template::InstancedStaticCall<math::vec3, math::vec3, math::vec3*>::callee<VecAdd>())
		.addFunction("__sub", Utils::Template::InstancedStaticCall<math::vec3, math::vec3, math::vec3*>::callee<VecSub>())
		.addFunction("__mod", Utils::Template::InstancedStaticCall<math::vec3, math::vec3, math::vec3*>::callee<VecCrossProduct>())
	.endClass()

	.beginClass<math::vec2>("vec2")
		.addConstructor<void(*)(float, float)>()
		.addData("x", &math::vec2::x)
		.addData("y", &math::vec2::y)
		.addProperty<math::vec2, math::vec2>("normalized", &VecNormalized<math::vec2>)
		.addFunction("length", &math::vec2::length)
		.addFunction("normalize", Utils::Template::InstancedStaticCall<math::vec2, void>::callee<VecNormalize>())
		.addFunction("__tostring", Utils::Template::InstancedStaticCall<math::vec2, std::string>::callee<Vec2ToString>())

		.addFunction("__mul", Utils::Template::InstancedStaticCall<math::vec2, float, math::vec2*>::callee<VecDotProduct>())
		.addFunction("__add", Utils::Template::InstancedStaticCall<math::vec2, math::vec2, math::vec2*>::callee<VecAdd>())
		.addFunction("__sub", Utils::Template::InstancedStaticCall<math::vec2, math::vec2, math::vec2*>::callee<VecSub>())
	.endClass()
	;
}

RegisterApiNonClass(ScriptMath, &ScriptMath, "math");

std::random_device _RandomDevice;
std::mt19937 _PseudoRandom(_RandomDevice());

void RandomNamespace(ApiInitializer &root) {
	struct T {
		static void Seed(int seed) {
			_PseudoRandom = std::mt19937(seed);
		}		
		static void Randomize() {
			_PseudoRandom = std::mt19937(_RandomDevice());
		}

		static int UniformRange(int min, int max) {
			std::uniform_int_distribution<int> generator(min, max);
			return generator(_PseudoRandom);
		}
		//static int NormalRange(int min, int max) {
			//std::normal_distribution<int> generator(min, max);
			//return generator(_PseudoRandom);
		//}

		static lua_Number UniformFloat() {
			std::uniform_real_distribution<lua_Number> generator;
			return generator(_PseudoRandom);
		}
		static lua_Number NormalFloat() {
			std::normal_distribution<lua_Number> generator(0, 1);
			return generator(_PseudoRandom);
		}

		static bool Boolean() {
			std::bernoulli_distribution dist;
			return dist(_PseudoRandom);
		}
	};
	root
	.addFunction("Seed", &T::Seed)
	.addFunction("Randomize", &T::Randomize)

	.addFunction("UniformRange", &T::UniformRange)
	//.addFunction("NormalRange", &T::NormalRange)

	.addFunction("UniformFloat", &T::UniformFloat)
	.addFunction("NormalFloat", &T::NormalFloat)

	.addFunction("Boolean", &T::Boolean)


	//.addFunction("randomf", &randomf)
	//.addFunction("vec4", &T::MakeVec4)
	//.addFunction("vec2", &T::MakeVec2)
	//.beginClass<math::vec3>("vec3")
	//	.addConstructor<void(*)(float, float, float)>()
	//	.addData("x", &math::vec3::x)
	//	.addData("y", &math::vec3::y)
	//	.addData("z", &math::vec3::z)
	//	.addFunction("length", &math::vec3::length)
	//.endClass()
	;
}
RegisterApiNonClass(RandomNamespace, &RandomNamespace, "random");

} //namespace Scripts 
} //namespace Core 
} //namespace MoonGlare 
