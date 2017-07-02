#include <pch.h>
#include <nfMoonGlare.h>

#include "../ScriptEngine.h"
#include "StaticModules.h"

namespace MoonGlare::Core::Scripts::Modules {
       
int randomi(int rmin, int rmax) {
    int d = rmax - rmin + 1;
    int r = (rand() % d) + rmin;
    // AddLog("random: " << rmin << "   " << r << "    " << rmax);
    return r;
}

int RandRange(int rmin, int rmax) {
    int d = rmax - rmin + 1;
    int r = (rand() % d) + rmin;
    // AddLog("random: " << rmin << "   " << r << "    " << rmax);
    return r;
}

float randomf(float rmin, float rmax) {
    float d = rmax - rmin;
    float r = (rand() % static_cast<int>(d)) + rmin;
    // AddLog("random: " << rmin << "   " << r << "    " << rmax);
    return r;
}

std::random_device _RandomDevice;
std::mt19937 _PseudoRandom(_RandomDevice());

void RandomNamespace(lua_State *lua) {
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
        static lua_Number NormalFloat() {   //gaussian
            std::normal_distribution<lua_Number> generator(0, 1);
            return generator(_PseudoRandom);
        }

        static bool Boolean() {
            std::bernoulli_distribution dist;
            return dist(_PseudoRandom);
        }
    };

    luabridge::getGlobalNamespace(lua)
    .beginNamespace("random")

    .addFunction("random", &randomi)
    .addFunction("RandRange", &randomi)

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

void StaticModules::InitRandom(lua_State *lua, World *world) {
    DebugLogf(Debug, "Initializing Random module");

    RandomNamespace(lua);
}

} //namespace MoonGlare::Core::Scripts::Modules
