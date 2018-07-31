#pragma once

#include "LuaBridge/LuaBridge.h"

#include <boost/tti/has_static_member_function.hpp>

namespace MoonGlare::Script {

using ApiInitializer = luabridge::Namespace;

using ApiInitFuncSignature = ApiInitializer(ApiInitializer);
using ApiInitFunc = ApiInitializer(*)(ApiInitializer);

namespace detail {
    BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(RegisterScriptApi)
}

template<typename T>
constexpr inline ApiInitFunc GetApiInitFunc() {
    using Has = detail::has_static_member_function_RegisterScriptApi<T, ApiInitFuncSignature>;
    if constexpr (Has::value) {
        return &T::RegisterScriptApi;
    } else
        return nullptr;
}

}
