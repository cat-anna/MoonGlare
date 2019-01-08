
local function dofmt(...)
    local n = select("#", ...);
    local inv = { ... }
    local out = { }
    for i=1,n do
        if inv[i] == nil then
            out[i] = "[NIL]"
        else        
            out[i] = tostring(inv[i])
        end
    end
    return table.concat(out, " ")
end

--[[@ [LuaPrintModule/_] Simple print functions
    Provides access to simple logging. This functions behave similar to lua print function:
    * `print(...)` 
    * `warning(...)` 
    * `hint(...)` 
    * `DebugPrint(...)` 
@]]
function print(...) Log.Console(dofmt(...)) end
function warning(...) Log.Warning(dofmt(...)) end
function hint(...) Log.Hint(dofmt(...)) end
function DebugPrint(...) Log.Debug(dofmt(...)) end


--[[@ [LuaPrintModule/_] Format print functions
    Provides access to simple logging. This functions behave similar to lua print function:
    * `printf(fmt, ...)` 
    * `warningf(fmt, ...)` 
    * `hintf(fmt, ...)` 
    * `DebugPrintf(fmt, ...)` 
@]]
function printf(...) Log.Console(string.format(...)) end
function warningf(...) Log.Warning(string.format(...)) end
function hintf(...) Log.Hint(string.format(...)) end
function DebugPrintf(...) Log.Debug(string.format(...)) end

--[[@ [LuaPrintModule/_] Error function `error(msg[, level])`  
    This is a wrapped lua error function. Additionally does logging.
@]]
local lua_error = error
function error(msg, c) 
    Log.Error(msg) 
    lua_error(t, (c or 0) + 1)
end
