#pragma once

#if __has_include(<luajit/lua.hpp>)
#include <luajit/lua.hpp>
#else
#include_next <lua.hpp>
#endif
