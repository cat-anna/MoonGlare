
local function dofmt(...)
    local out = { }
    for i,v in ipairs({...}) do
        if v == nil then
            out[i] = "[NIL]"
        else        
            out[i] = tostring(v)
        end
    end
    return table.concat(out, " ")
end

function print(...) Log.Console(dofmt(...)) end
function warning(...) Log.Warning(dofmt(...)) end
function hint(...) Log.Hint(dofmt(...)) end
function DebugPrint(...) Log.Debug(dofmt(...)) end

function printf(...) Log.Console(string.format(...)) end
function warningf(...) Log.Warning(string.format(...)) end
function hintf(...) Log.Hint(string.format(...)) end
function DebugPrintf(...) Log.Debug(string.format(...)) end

local lua_error = error
function error(msg, c) 
    Log.Error(msg) 
    lua_error(t, (c or 0) + 1)
end
