#pragma once

#include "ErrorHandling.h"

#define LuaRunError(lua, Reason, DetailsFmt, ...)                                                           \
do {                                                                                                        \
    auto msg = fmt::format("{} : Error : {} : " DetailsFmt, ScriptFunctionName, Reason, __VA_ARGS__ );      \
    lua_pushstring(lua, msg.c_str());                                                                       \
    lua_error(lua);                                                                                         \
} while (false) 

#define LuaRunWarning(lua, Reason, DetailsFmt, ...)                                                         \
do {                                                                                                        \
    auto msg = fmt::format("{} : Warning : {} : " DetailsFmt, ScriptFunctionName, Reason, __VA_ARGS__ );    \
    if(lua) { lua_pushstring(lua, msg.c_str()); LuaTraceback(lua); }                                        \
} while (false) 

#define ReportMissingFunction(lua, FuncName)                                                                \
do {                                                                                                        \
    auto msg = fmt::format("System : Warning : Missing function : Function '{}' is not defined", FuncName );\
    AddLogf(ScriptRuntime, msg.c_str());                                                                    \
} while (false)                                                                                             \


#define LuaReportInvalidArg(lua, index, expectedType)                                                       \
do {                                                                                                        \
    LuaRunError(lua, "Invalid argument #" ## BOOST_STRINGIZE(index),                                        \
        " Invalid argument #{} type. Expected {} got {}",                                                   \
        index, BOOST_STRINGIZE(expectedType), lua_typename(lua, lua_type(lua, -1)));                        \
} while (false)                                                                                             \

