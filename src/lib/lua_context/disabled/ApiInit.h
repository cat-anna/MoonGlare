#pragma once

#include "LuaBridge/LuaBridge.h"

#include <boost/tti/has_static_member_function.hpp>

namespace MoonGlare::Scripts {

using ClassKey = const void*;

using ApiInitializer = luabridge::Namespace;

using ApiInitFuncSignature = ApiInitializer(ApiInitializer);
using ApiInitFunc = ApiInitializer(*)(ApiInitializer);

namespace detail {
BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(RegisterScriptApi);
BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(RegisterDebugScriptApi);
}

template<typename T>
constexpr inline ApiInitFunc GetApiInitFunc() {
    using Has = detail::has_static_member_function_RegisterScriptApi<T, ApiInitFuncSignature>;
    if constexpr (Has::value) {
        return &T::RegisterScriptApi;
    } else
        return nullptr;
}

template<typename T>
constexpr inline ApiInitFunc GetDebugApiInitFunc() {
#ifdef DEBUG_SCRIPTAPI
    using Has = detail::has_static_member_function_RegisterDebugScriptApi<T, ApiInitFuncSignature>;
    if constexpr (Has::value) {
        return &T::RegisterDebugScriptApi;
    }
    else
        return nullptr;
#else
    return nullptr;
#endif
}

}
