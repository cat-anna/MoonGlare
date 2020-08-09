#include <cmath>
#include <functional>
#include "Math.h"
#include "StaticModules.h"

#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btQuaternion.h>
#include <bullet/LinearMath/btTransform.h>
#include <glm/glm.hpp>
#include <eigen3/Eigen/Core>

#include <Math/libMath.h>
#include <Foundation/TemplateUtils.h>

namespace MoonGlare::Scripts::Modules {

#define NN(a)   \
if (!a)    __debugbreak();
//throw std::runtime_error(__FUNCTION__ " - nullptr! (" #a ")");

//Other

template<int multiplier = 1,int divider = 1>
float getPi() { return 3.14159f * (static_cast<float>(multiplier) / static_cast<float>(divider)); }

//Templates

template<typename T> inline T VecNormalized(const T * vec) { return glm::normalize(*vec); }
template<typename T> inline void VecNormalize(T *vec) { *vec = glm::normalize(*vec); }
template<typename T> inline float VecLength(const T *vec) { return glm::length(*vec); }
template<typename T> inline float VecDotProduct(T *a, T* b) { return glm::dot(*a, *b); }
template<typename T> inline T VecCrossProduct(T *a, T* b) { return glm::cross(*a, *b); }
template<typename T> inline T VecDiv(T *a, T* b) { NN(a) NN(b) return *a / *b; }
template<typename T> inline T VecMul(T *a, T* b) { NN(a) NN(b) return *a * *b; }
template<typename T> inline T VecAdd(T *a, T* b) { NN(a) NN(b) return *a + *b; }
template<typename T> inline T VecSub(T *a, T* b) { NN(a) NN(b) return *a - *b; }
template<typename T> inline std::string ToString(T *vec) {
    std::ostringstream oss;
    oss << *vec;
    return oss.str();
}
template<typename T, typename A, int ... ints> inline T StaticVec() { return T(static_cast<A>(ints)...); }
template<typename T> inline T VecClamp(const T &v, const T &min, const T &max) {
    //NN(v); NN(min); NN(max);
    T ret = v;
    for (int i = 0; i < ret.length(); ++i) {
        if (ret[i] < min[i])
            ret[i] = min[i];
        else
            if (ret[i] > max[i])
                ret[i] = max[i];
    }
    return ret;
}
template<typename T> inline void VecClampSelf(T *v, const T &min, const T &max) { *v = VecClamp(*v, min, max); }

//Quaternions/Vec4

inline math::vec3 QuatRotateVec(math::vec4 *a, math::vec3 *b) {
    auto q = emath::MathCast<emath::Quaternion>(*a);
    emath::Quaternion vec = { 0.0f, b->x, b->y, b->z };
    emath::Quaternion r = q * vec * q.inverse();
    return { r.x(), r.y(), r.z() };
}
inline math::vec4 QuaternionCrossProduct(math::vec4 *a, math::vec4* b) {
    auto &q1 = *a;
    auto &q2 = *b;
    math::vec4 out;
    out.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y;
    out.y = q1.w*q2.y + q1.y*q2.w + q1.z*q2.x - q1.x*q2.z;
    out.z = q1.w*q2.z + q1.z*q2.w + q1.x*q2.y - q1.y*q2.x;
    out.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
    return out;
}
template<> inline std::string ToString<math::vec4>(math::vec4 *vec) {
    auto &t = *vec;
    char b[256];
    sprintf(b, "Vec4(%f, %f, %f, %f)", t[0], t[1], t[2], t[3]);
    return b;
}
inline math::vec4 QuaternionRotationTo(const math::vec3 &a, const math::vec3 &b) {
    // Based on Stan Melax's article in Game Programming Gems
    math::Quaternion q;
    // Copy, since cannot modify local
    btVector3 v0 = convert(a);
    btVector3 v1 = convert(b);

    v0.normalize();
    v1.normalize();

    float d = v0.dot(v1);
    // If dot == 1, vectors are the same
    if (d >= 1.0f) {
        return convert(math::Quaternion::getIdentity());
    }
    if (d < (1e-6f - 1.0f)) {
        if (v1 != btVector3(0, 0, 0)) {
            // rotate 180 degrees about the fallback axis
            q = math::Quaternion(v1, glm::radians(180.0f));
        } else {
            // Generate an axis
            btVector3  axis = btVector3(1, 0, 0).cross(v0);
            if (axis.length2() < 1e-06f * 1e-06f) // pick another if colinear
                axis = btVector3(0, 1, 0).cross(v0);
            axis.normalize();
            q = math::Quaternion(axis, glm::radians(180.0f));
        }
    } else {
        float s = sqrtf((1 + d) * 2);
        float invs = 1 / s;

        btVector3 c = v0.cross(v1);

        c *= invs;
        q = math::Quaternion(c.getX(), c.getY(), c.getZ(), s * 0.5f);
        q.normalize();
    }
    return convert(q);
}
inline math::vec4 QuaternionFromVec3Angle(const math::vec3 &vec, float a) {
    float halfAngle = a * .5f;
    float s = (float)sin(halfAngle);
    return math::vec4(vec * s, cos(halfAngle));
}
inline math::vec4 QuaternionFromEulerXYZ(float x, float y, float z) {
    return convert(math::Quaternion(x, y, z));
}
inline math::vec4 QuaternionFromAxisAngle(float x, float y, float z, float a) {
    return QuaternionFromVec3Angle(math::vec3(x, y, z), a);
}
inline math::vec4 QuaternionLookAt(const math::vec3 &sourcePoint, const math::vec3 &destPoint) {
    //source https://stackoverflow.com/questions/12435671/quaternion-lookat-function

    math::vec3 forwardVector = glm::normalize(destPoint - sourcePoint);

    float dot = glm::dot(
        //Vector3.forward
        math::fvec3(1, 0, 0)
        , forwardVector);

    if (abs(dot - (-1.0f)) < 0.000001f) {
        return math::vec4(math::fvec3(1, 0, 0), emath::constant::pi<float>);
    }
    if (abs(dot - (1.0f)) < 0.000001f) {
        return { 0,0,0,1 };
    }

    float rotAngle = (float)acos(dot);
    math::vec3 rotAxis = glm::cross(
        //Vector3.forward
        math::fvec3(1, 0, 0), forwardVector);

    rotAxis = glm::normalize(rotAxis);
    return QuaternionFromVec3Angle(rotAxis, rotAngle);
#if 0
    /// <summary>
    /// Evaluates a rotation needed to be applied to an object positioned at sourcePoint to face destPoint
    /// </summary>
    /// <param name="sourcePoint">Coordinates of source point</param>
    /// <param name="destPoint">Coordinates of destionation point</param>
    /// <returns></returns>
    public static Quaternion LookAt(Vector3 sourcePoint, Vector3 destPoint) {
        Vector3 forwardVector = Vector3.Normalize(destPoint - sourcePoint);

        float dot = Vector3.Dot(Vector3.forward, forwardVector);

        if (Math.Abs(dot - (-1.0f)) < 0.000001f) {
            return new Quaternion(Vector3.up.x, Vector3.up.y, Vector3.up.z, 3.1415926535897932f);
        }
        if (Math.Abs(dot - (1.0f)) < 0.000001f) {
            return Quaternion.identity;
        }

        float rotAngle = (float)Math.Acos(dot);
        Vector3 rotAxis = Vector3.Cross(Vector3.forward, forwardVector);
        rotAxis = Vector3.Normalize(rotAxis);
        return CreateFromAxisAngle(rotAxis, rotAngle);
    }
#endif
}
inline int lua_NewQuaternion(lua_State *lua) {
    int argc = lua_gettop(lua);
    switch (argc) {
    case 1:
        luabridge::Stack<math::vec4>::push(lua, math::vec4());
        return 1;
    case 2:
        if (lua_isnumber(lua, -1)) {
            luabridge::Stack<math::vec4>::push(lua, math::vec4(static_cast<float>(lua_tonumber(lua, -1))));
            return 1;
        }
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        if (lua_isnumber(lua, 2) && lua_isnumber(lua, 3) && lua_isnumber(lua, 4) && lua_isnumber(lua, 5)) {
            luabridge::Stack<math::vec4>::push(lua,
                                               math::vec4(
                                                   static_cast<float>(lua_tonumber(lua, 2)),
                                                   static_cast<float>(lua_tonumber(lua, 3)),
                                                   static_cast<float>(lua_tonumber(lua, 4)),
                                                   static_cast<float>(lua_tonumber(lua, 5))));
            return 1;
        }
    default:
        break;
    }
    AddLog(Error, "Invalid arguments!");
    lua_pushnil(lua);
    return 1;
}

//Vec3

inline math::vec3 Vec3CrossProduct(math::vec3 *a, math::vec3* b) { return glm::cross(*a, *b); }
template<> inline std::string ToString<math::vec3>(math::vec3 *vec) {
    auto &t = *vec;
    char b[256];
    sprintf(b, "Vec3(%f, %f, %f)", t[0], t[1], t[2]);
    return b;
}
inline int lua_NewVec3(lua_State *lua) {
    int argc = lua_gettop(lua);
    switch (argc) {
    case 1:
        luabridge::Stack<math::vec3>::push(lua, math::vec3());
        return 1;
    case 2:
        if (lua_isnumber(lua, -1)) {
            luabridge::Stack<math::vec3>::push(lua, math::vec3(static_cast<float>(lua_tonumber(lua, -1))));
            return 1;
        }

    case 3:
        break;
    case 4:
        if (lua_isnumber(lua, -3) && lua_isnumber(lua, -2) && lua_isnumber(lua, -1)) {
            luabridge::Stack<math::vec3>::push(lua,
                                               math::vec3(
                                                   static_cast<float>(lua_tonumber(lua, -3)),
                                                   static_cast<float>(lua_tonumber(lua, -2)),
                                                   static_cast<float>(lua_tonumber(lua, -1))));
            return 1;
        }
    default:
        break;
    }
    AddLog(Error, "Invalid arguments!");
    lua_pushnil(lua);
    return 1;
}
inline math::vec3 SphericalToCartesian(const math::vec2 *vec) {
    NN(vec)
    float th = vec->y;
    float fi = vec->x;
    return glm::normalize(math::vec3(
        sinf(th) * cosf(fi)
        ,
        sinf(th) * sinf(fi)
        ,
        cos(th)
    ));
}

//Vec2

template<> inline std::string ToString<math::vec2>(math::vec2 *vec) {
    auto &t = *vec;
    char b[256];
    sprintf(b, "vec2(%f, %f)", t[0], t[1]);
    return b;
}
inline int lua_NewVec2(lua_State *lua) {
    int argc = lua_gettop(lua);
    switch (argc) {
    case 1:
        luabridge::Stack<math::vec2>::push(lua, math::vec2());
        return 1;

    case 2:
        if (lua_isnumber(lua, -1)) {
            luabridge::Stack<math::vec2>::push(lua, math::vec2(static_cast<float>(lua_tonumber(lua, -1))));
            return 1;
        }

    case 3:
        if (lua_isnumber(lua, -2) && lua_isnumber(lua, -1)) {
            luabridge::Stack<math::vec2>::push(lua,
                                               math::vec2(
                                                   static_cast<float>(lua_tonumber(lua, -2)),
                                                   static_cast<float>(lua_tonumber(lua, -1))));
            return 1;
        }
    default:
        break;
    }
    AddLog(Error, "Invalid arguments!");
    lua_pushnil(lua);
    return 1;
}

//Linear

inline float Clamp(float v, float min, float max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}
inline float Clamp01(float v) { return Clamp(v, 0.0f, 1.0f); }

//Common

/*@ [MathModule/VecCommon] Vec4/Vec3/Vec2 share some commion api @*/            
template<typename VEC>
struct VecCommon {
    template<typename API>
    static void f(API &api) {
        api
/*@ [VecCommon/_] vec.Normalized 
    Get vector normalized  value @*/            
            .addProperty<VEC>("Normalized", &VecNormalized<VEC>)
/*@ [VecCommon/_] vec.Length 
    Get vector length  value @*/                
            .addProperty<float>("Length", &VecLength<VEC>)

/*@ [VecCommon/_] vec:Normalize() 
    Normalize vector in place@*/       
            .addFunction("Normalize", Utils::Template::InstancedStaticCall<VEC, void>::callee<VecNormalize>())
            
/*@ [VecCommon/_] vec:Clamp(min, max) 
    Clamp vector values in place @*/             
            .addFunction("Clamp", Utils::Template::InstancedStaticCall<VEC, void, const VEC&, const VEC&>::callee<VecClampSelf>())

/*@ [VecCommon/_] Overloaded operators
    Each vector object have overloaded operators: * + - one can use them to perform calculations.  
    Also `tostring(vev)` is overloaded, all vectors are nicely converted to string @*/     
            .addFunction("__tostring", Utils::Template::InstancedStaticCall<VEC, std::string>::callee<ToString>())
            .addFunction("__mul", Utils::Template::InstancedStaticCall<VEC, VEC, VEC*>::callee<VecMul>())
            .addFunction("__add", Utils::Template::InstancedStaticCall<VEC, VEC, VEC*>::callee<VecAdd>())
            .addFunction("__sub", Utils::Template::InstancedStaticCall<VEC, VEC, VEC*>::callee<VecSub>())
            ;
    }
};

//Registration

void ScriptMathClasses(lua_State *lua) {
    luabridge::getGlobalNamespace(lua)
/*@ [MathModule/MathLibExtension] Extensions to lua math library
    TODO @*/      
    .beginNamespace("math")
/*@ [MathLibExtension/_] `math.Clamp(value, min, max)`
    TODO @*/     
        .addFunction("Clamp", &Clamp)
/*@ [MathLibExtension/_] `math.Clamp01(value)`
    TODO @*/             
        .addFunction("Clamp01", &Clamp01)
    //	.addProperty("pi", &getPi)
    //	.addProperty("pi_half", &getPi<1, 2>)

/*@ [MathModule/Vec4Reference] Vec4/Quaternion object reference
    TODO @*/    
        .beginClass<math::vec4>("cVec4")
/*@ [Vec4Reference/_] Members
    `vec4.x`, `vec4.y`, `vec4.z`, `vec4.w` - access corresponding value @*/            
            .addData("x", &math::vec4::x)
            .addData("y", &math::vec4::y)
            .addData("z", &math::vec4::z)
            .addData("w", &math::vec4::w)
/*@ [Vec4Reference/_] `vec4:dot(vec4)`
    Get vector dot product @*/            
            .addFunction("dot", Utils::Template::InstancedStaticCall<math::vec4, float, math::vec4*>::callee<VecDotProduct>())
/*@ [Vec4Reference/_] `vec4:cross(vec4)`
    Get vector cross product @*/                
            .addFunction("cross", Utils::Template::InstancedStaticCall<math::vec4, math::vec4, math::vec4*>::callee<QuaternionCrossProduct>())
/*@ [Vec4Reference/_] `vec4:rotate(vec3)`
    TODO @*/                 
            .addFunction("rotate", Utils::Template::InstancedStaticCall<math::vec4, math::vec3, math::vec3*>::callee<QuatRotateVec>())
            .DefferCalls<&VecCommon<math::vec4>::f>()
        .endClass()
/*@ [MathModule/Vec3Reference] Vec3 object reference
    TODO @*/            
        .beginClass<math::vec3>("cVec3")
/*@ [Vec3Reference/_] Members
    `vec3.x`, `vec3.y`, `vec3.z` - access corresponding value @*/             
            .addData("x", &math::vec3::x)
            .addData("y", &math::vec3::y)
            .addData("z", &math::vec3::z)
/*@ [Vec3Reference/_] `vec3:dot(vec3)`
    Get vector dot product @*/                      
            .addFunction("dot", Utils::Template::InstancedStaticCall<math::vec3, float, math::vec3*>::callee<VecDotProduct>())
/*@ [Vec3Reference/_] `vec3:cross(vec3)`
    Get vector cross product @*/                 
            .addFunction("cross", Utils::Template::InstancedStaticCall<math::vec3, math::vec3, math::vec3*>::callee<VecCrossProduct>())
            .DefferCalls<&VecCommon<math::vec3>::f>()
        .endClass()
/*@ [MathModule/Vec2Reference] Vec2 object reference
    TODO @*/         
        .beginClass<math::vec2>("cVec2")
/*@ [Vec2Reference/_] Members
    `vec2.x`, `vec2.y` - access corresponding value @*/                 
            .addData("x", &math::vec2::x)
            .addData("y", &math::vec2::y)
            .DefferCalls<&VecCommon<math::vec2>::f>()
        .endClass()
    .endNamespace()
    ;
}

//-------------------------------------------------------------------------------------------------

void ScriptMathGlobal(lua_State *lua) {
    luabridge::getGlobalNamespace(lua)

/*@ [MathModule/NamespaceQuaternion] Quaternion namespace
    Global `Quaternion` namespace groups methods related only to quaternions.  
    However internally vec4 and Quaternion types are equivalent. @*/    

    .beginNamespace("Quaternion")
/*@ [NamespaceQuaternion/_] `Quaternion([x [, y , z , w]])`
    Creates new quaternion object. Created quaternion depend on count of arguments.
    * 0 -> all 4 values are set to 0
    * 1 -> all 4 values of object are set to x
    * 2,3 -> not allowed
    * 4 -> all 4 values are used
    NOTE: created quaternion is not normalized @*/   
        .addCFunction("__call", &lua_NewQuaternion)

/*@ [NamespaceQuaternion/_] `Quaternion.FromVec3Angle(x, y, z, angle)`
    TODO @*/            
        .addFunction("FromAxisAngle", &QuaternionFromAxisAngle)

/*@ [NamespaceQuaternion/_] `Quaternion.FromVec3Angle(vec3, angle)`
    TODO @*/            
        .addFunction("FromVec3Angle", &QuaternionFromVec3Angle)

/*@ [NamespaceQuaternion/_] `Quaternion.FromEulerXYZ(z, y, x)`
    TODO @*/            
        .addFunction("FromEulerXYZ", &QuaternionFromEulerXYZ)

/*@ [NamespaceQuaternion/_] `Quaternion.RotationTo(vec3, vec3)`
    TODO @*/            
        .addFunction("RotationTo", &QuaternionRotationTo)

/*@ [NamespaceQuaternion/_] `Quaternion.LookAt(vec3_sourcePoint, vec3_destPoint)`
    TODO @*/            
        .addFunction("LookAt", &QuaternionLookAt)

/*@ [NamespaceQuaternion/_] `Quaternion.Identity`
    Return identity quaternion @*/    
        .addProperty("Identity", &StaticVec<math::vec4, float, 0, 0, 0, 1>)  
    .endNamespace()

/*@ [MathModule/NamespaceVec4] Vec4 namespace
    Global `Vec4` namespace groups methods related only to 4 element vector.  
    However internally vec4 and Quaternion types are equivalent. @*/   
    .beginNamespace("Vec4")

/*@ [NamespaceQuaternion/_] `Vec4([x [, y , z , w]])`
    Behaves exactly the same as Quaternion([x [, y , z , w]]) @*/       
        .addCFunction("__call", &lua_NewQuaternion)

/*@ [NamespaceQuaternion/_] `Vec4.Clamp(vec4, vec4_min, vec4_max)`
    Clamps each vec4 element between vec4_min and vec4_max. @*/            
        .addFunction("Clamp", &VecClamp<math::vec4>)

/*@ [NamespaceQuaternion/_] Static values
    Namespace `Vec4` has several static values:
    * `Vec4.X`, `Vec4.Y`, `Vec4.Z`, `Vec4.W` - returned vector contain single 1 in specified axis
    * `Vec4.R`, `Vec4.G`, `Vec4.B` - returned vector contain single 1 in specified color and 1 as alpha value
    * `Vec4.White`, `Vec4.Black` - four 1's or 0's
    * `Vec4.One`, `Vec4.Zero` - four 1's or 0's @*/            
        .addProperty("X", &StaticVec<math::vec4, float, 1, 0, 0, 0>)
        .addProperty("Y", &StaticVec<math::vec4, float, 0, 1, 0, 0>)
        .addProperty("Z", &StaticVec<math::vec4, float, 0, 0, 1, 0>)
        .addProperty("W", &StaticVec<math::vec4, float, 0, 0, 0, 1>)

        .addProperty("R", &StaticVec<math::vec4, float, 1, 0, 0, 1>)
        .addProperty("G", &StaticVec<math::vec4, float, 0, 1, 0, 1>)
        .addProperty("B", &StaticVec<math::vec4, float, 0, 0, 1, 1>)
        .addProperty("White", &StaticVec<math::vec4, float, 1, 1, 1, 1>)
        .addProperty("Black", &StaticVec<math::vec4, float, 0, 0, 0, 1>)

        .addProperty("Zero", &StaticVec<math::vec4, float, 0, 0, 0, 0>)
        .addProperty("One", &StaticVec<math::vec4, float, 1, 1, 1, 1>)
    .endNamespace()

/*@ [MathModule/NamespaceVec3] Vec3 namespace
    Global `Vec3` namespace groups methods related only to 3 element vector. @*/   
    .beginNamespace("Vec3")
/*@ [NamespaceVec3/_] `Vec3([x [, y , z]])`
    Creates new vec3 object. Created object depend on count of arguments.
    * 0 -> all 3 values are set to 0
    * 1 -> all 3 values of object are set to x
    * 2 -> not allowed
    * 3 -> all 3 values are used @*/       
        .addCFunction("__call", &lua_NewVec3)

/*@ [NamespaceVec3/_] `Vec3.FromSpherical(vec2)`
    TODO @*/               
        .addFunction("FromSpherical", &SphericalToCartesian)
/*@ [NamespaceVec3/_] `Vec3.Clamp(vec3, vec3_min, vec3_max)`
    TODO @*/                       
        .addFunction("Clamp", &VecClamp<math::vec3>)

/*@ [NamespaceVec3/_] Static values
    Namespace `Vec3` has several static values:
    * `Vec3.Zero`, `Vec3.One` - all 0's or 1's
    * `Vec3.Up`, `Vec3.Down`, `Vec3.Forward`, `Vec3.Backward`, `Vec3.Left`, `Vec3.Right` - 1 or -1 in proper axis
    * `Vec3.X`, `Vec3.Y`, `Vec3.Z` - returned vector contain single 1 in specified axis @*/
        .addProperty("Zero", &StaticVec<math::vec3, float, 0, 0, 0>)
        .addProperty("One", &StaticVec<math::vec3, float, 1, 1, 1>)

        .addProperty("Up", &StaticVec<math::vec3, float, 0, 0, 1>)
        .addProperty("Down", &StaticVec<math::vec3, float, 0, 0, -1>)

        .addProperty("Forward", &StaticVec<math::vec3, float, 1, 0, 0>)
        .addProperty("Backward", &StaticVec<math::vec3, float, -1, 0, 0>)

        .addProperty("Left", &StaticVec<math::vec3, float, 0, 1, 0>)
        .addProperty("Right", &StaticVec<math::vec3, float, 0, -1, 0>)

        .addProperty("X", &StaticVec<math::vec3, float, 1, 0, 0>)
        .addProperty("Y", &StaticVec<math::vec3, float, 0, 1, 0>)
        .addProperty("Z", &StaticVec<math::vec3, float, 0, 0, 1>)
    .endNamespace()
        
/*@ [MathModule/NamespaceVec2] Vec2 namespace
    Global `Vec2` namespace groups methods related only to 2 element vector. @*/           
    .beginNamespace("Vec2")
/*@ [NamespaceVec2/_] `Vec2([x [, y , z]])`
    Creates new vec2 object. Created object depend on count of arguments.
    * 0 -> all 2 values are set to 0
    * 1 -> all 2 values of object are set to x
    * 2 -> all 2 values are used @*/       
        .addCFunction("__call", &lua_NewVec2)

/*@ [NamespaceVec2/_] `Vec2.Clamp(vec2, vec2_min, vec2_max)`
    TODO @*/             
        .addFunction("Clamp", &VecClamp<math::vec2>)

/*@ [NamespaceVec2/_] Static values
    Namespace `Vec2` has several static values:
    * `Vec2.Zero`, `Vec2.One` - all 0's or 1's
    * `Vec2.X`, `Vec2.Y` - returned vector contain single 1 in specified axis @*/
        .addProperty("Zero", &StaticVec<math::vec2, float, 0, 0>)
        .addProperty("One", &StaticVec<math::vec2, float, 1, 1>)

        .addProperty("X", &StaticVec<math::vec2, float, 1, 0>)
        .addProperty("Y", &StaticVec<math::vec2, float, 0, 1>)
    .endNamespace()
    ;
}

//-------------------------------------------------------------------------------------------------

/*@ [StaticModules/MathModule] Math module
    Standard lua math library is available, but it does not support vector operations.  
    To address this issue few additional global namespaces are available and dedicated vector classes.
@*/

void InitLuaMath(lua_State *lua) {
    DebugLogf(Debug, "Initializing Math module");
    ScriptMathGlobal(lua);
    ScriptMathClasses(lua);
}

} //namespace MoonGlare::Scripts::Modules