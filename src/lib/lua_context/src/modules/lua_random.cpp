#include "lua_context/modules/lua_random.hpp"
#include "lua_context_build_config.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>
#include <random>
#include <sol/sol.hpp>

namespace MoonGlare::Lua {

namespace {

struct RandomObject {
    RandomObject() : mt(0) {}
    void SetSeed(int value) { mt.seed(value); }
    void Randomize() { mt.seed(std::random_device()()); }

    bool nextBool() { return boolDist(mt); }
    double nextDouble() { return realDist(mt); }
    double nextDoubleRange(double a, double b) {
        std::uniform_real_distribution<double> d{a, b};
        return d(mt);
    }
    double nextGaussian() { return normalDist(mt); }
    int nextInt() { return intDist(mt); }
    int nextIntRange(int a, int b) {
        std::uniform_int_distribution<int> d(a, b);
        return d(mt);
    }

    std::mt19937 mt;
    std::uniform_real_distribution<double> realDist{0.0, 1.0};
    std::uniform_int_distribution<int> intDist{std::numeric_limits<int>::min(), std::numeric_limits<int>::max()};
    std::normal_distribution<double> normalDist{0, 1};
    std::bernoulli_distribution boolDist{0.5};

    static RandomObject Create() { return {}; }
};

struct RandomModule {
    RandomObject Create() { return RandomObject{}; }
};

} // namespace

/*@ [LuaModules/RandomModule] Random module
    Provides various methods of generation random values
@*/

LuaRandomModule::LuaRandomModule() : iDynamicScriptModule("LuaRandomModule"), iRequireRequest("random") {
}

bool LuaRandomModule::OnRequire(lua_State *lua, std::string_view name) {
    sol::stack::push(lua, RandomModule{});
    return true;
}

void LuaRandomModule::InitContext(lua_State *lua) {
    sol::state_view sol_view(lua);
    auto ns = sol_view[kInternalLuaNamespaceName].get_or_create<sol::table>();

    /*@ [RandomModule/RandomObject] Get next value functions family
    This functions generates next random value.
    * `Random.next_bool()` - true/false, probability 0.5
    * `Random.next_float()` - [0-1) uniform distribution
    * `Random.next_double_range(a, b)` - [a-b) uniform distribution
    * `Random.next_gaussian()` - [0-1) normal distribution
    * `Random.next_int()` - full int range, normal distribution
    * `Random.next_int_range(a, b)` - [a-b) uniform distribution
    * `Random.set_seed(TODO)` - TODO
    * `Random.get_seed(TODO)` - TODO
    * `Random.randomize(TODO)` - TODO
@*/
    ns.new_usertype<RandomObject>("RandomObject",                                     //
                                  "next_bool", &RandomObject::nextBool,               //
                                  "next_float", &RandomObject::nextDouble,            //
                                  "next_gaussian", &RandomObject::nextGaussian,       //
                                  "next_float_range", &RandomObject::nextDoubleRange, //
                                  "next_int", &RandomObject::nextInt,                 //
                                  "next_int_range", &RandomObject::nextIntRange,      //
                                  "set_seed", &RandomObject::SetSeed,                 //
                                  "get_seed", &RandomObject::SetSeed,                 //
                                  "randomize", &RandomObject::Randomize               //
    );

    /*@ [RandomModule/_] `Random.new()`
    Create and return new RandomObject. All created objects share same seed.
@*/
    ns.new_usertype<RandomModule>("LuaRandomModule",           //
                                  "new", &RandomModule::Create //
    );
}

} // namespace MoonGlare::Lua
