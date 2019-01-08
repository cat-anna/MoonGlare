#include "StaticModules.h"

namespace MoonGlare::Scripts::Modules {

struct RandomObject {
    RandomObject() : mt(std::random_device()()) { }
    void SetSeed(int value) { mt.seed(value); }
    void Randomize() { mt.seed(std::random_device()()); }

    bool nextBool() { return boolDist(mt); }
    double nextDouble() { return realDist(mt); }
    double nextDoubleRange(double a, double b) { 
        std::uniform_real_distribution<double> d{ a, b };
        return d(mt);
    }
    double nextGaussian() { return normalDist(mt); }
    int nextInt() { return intDist(mt); }
    int nextIntRange(int a, int b) { 
        std::uniform_int_distribution<int> d(a, b);
        return d(mt); 
    }

    std::mt19937 mt;
    std::uniform_real_distribution<double> realDist{ 0.0, 1.0 };
    std::uniform_int_distribution<int> intDist{ std::numeric_limits<int>::min(), std::numeric_limits<int>::max() };
    std::normal_distribution<double> normalDist{0, 1};
    std::bernoulli_distribution boolDist{ 0.5 };

    static RandomObject Create() { return {}; }
};

static RandomObject randomDevice;

/*@ [StaticModules/RandomModule] Random module
    Available through `Random` global variable.
@*/

void RandomNamespace(lua_State *lua) {
    luabridge::getGlobalNamespace(lua)
        .beginNamespace("Random")

/*@ [RandomModule/_] Get next value functions family
    This functions generates next random value.
    * `Random.nextBool()` - true/false, probability 0.5
    * `Random.nextFloat()` - [0-1) uniform distribution
    * `Random.nextFloatRange(a, b)` - [a-b) uniform distribution
    * `Random.nextGaussian()` - [0-1) normal distribution
    * `Random.nextInt()` - full int range, normal distribution
    * `Random.nextIntRange(a, b)` - [a-b) uniform distribution
@*/
            .addObjectFunction("nextBool", &randomDevice, &RandomObject::nextBool)
            .addObjectFunction("nextFloat", &randomDevice, &RandomObject::nextDouble)
            .addObjectFunction("nextFloatRange", &randomDevice, &RandomObject::nextDoubleRange)
            .addObjectFunction("nextGaussian", &randomDevice, &RandomObject::nextGaussian)
            .addObjectFunction("nextInt", &randomDevice, &RandomObject::nextInt)
            .addObjectFunction("nextIntRange", &randomDevice, &RandomObject::nextIntRange)

/*@ [RandomModule/_] Seed control methods
    This functions influence  next generated random value
    * `Random.SetSeed(value)` - set seed value.
    * `Random.Randomize()` - set random seed
@*/
            .addObjectFunction("SetSeed", &randomDevice, &RandomObject::SetSeed)
            .addObjectFunction("Randomize", &randomDevice, &RandomObject::Randomize)

/*@ [RandomModule/_] `Random.New()`
    Create and return new Random object.   
    Random object share the same set of function as global Random object.  
    However they do not share the internal state and generation from one does not influence others.  
@*/
            .addFunction("New", &RandomObject::Create)
        .endNamespace()
        .beginNamespace("api")
            .beginClass<RandomObject>("cRandomObject")
                .addFunction("nextBool", &RandomObject::nextBool)
                .addFunction("nextDouble", &RandomObject::nextDouble)
                .addFunction("nextDoubleRange", &RandomObject::nextDoubleRange)
                .addFunction("nextGaussian", &RandomObject::nextGaussian)
                .addFunction("nextInt", &RandomObject::nextInt)
                .addFunction("nextIntRange", &RandomObject::nextIntRange)

                .addFunction("SetSeed", &RandomObject::SetSeed)
                .addFunction("Randomize", &RandomObject::Randomize)
            .endClass()
        .endNamespace()
        ;
}

void InitLuaRandom(lua_State *lua) {
    DebugLogf(Debug, "Initializing Random module");

    RandomNamespace(lua);
}

} //namespace MoonGlare::Scripts::Modules
