
local _log = log

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

--[[@ [BaseDataModule_Scripts/Log_Formatting] Log formatting
@]]

--[[@ [Log_Formatting/_] Simple print functions
Provides access to simple logging. This functions behave similar to lua print function:

* `print(...)`
* `warning(...)`
* `info(...)`
* `DebugPrint(...)`

@]]
function print(...) _log.info(dofmt(...)) end
function warning(...) _log.warning(dofmt(...)) end
function info(...) _log.info(dofmt(...)) end
function DebugPrint(...) _log.debug(dofmt(...)) end

--[[@ [Log_Formatting/_] Format print functions
Provides access to simple logging. This functions behave similar to lua print function:

* `printf(fmt, ...)`
* `warningf(fmt, ...)`
* `infof(fmt, ...)`
* `DebugPrintf(fmt, ...)`

@]]
function printf(...) _log.info(string.format(...)) end
function warningf(...) _log.warning(string.format(...)) end
function infof(...) _log.info(string.format(...)) end
function DebugPrintf(...) _log.debug(string.format(...)) end

--[[@ [Log_Formatting/_] Error function `error(msg[, level])`
This is a wrapped lua error function. Additionally does _logging.
@]]
local lua_error = error
function error(msg, c)
    _log.error(msg)
    lua_error(msg, (c or 0) + 1)
end
