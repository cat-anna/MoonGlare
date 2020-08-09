
local org_require = require
     
require = {  }
require.__index = require
require.__call = function(self, ...) return org_require(...) end
require.cache = require_cache
require_cache = nil

--[[@ [RequireModule/_] `require.reload(name)`
    Reloads code of single module. All objects which share class returned by its script file 
    will now use new one.  
    TODO: there are OnBeforeReload/OnAfterReload callbacks. Check how to use them or delete.
@]]
function require.reload(name)
    local req = require(name)
    local new = dofile(name)

    if type(req.OnBeforeReload) == "function" then
        req:OnBeforeReload()        
    end

    for k,v in pairs(new) do
        req[k] = v
    end

    if type(req.OnAfterReload) == "function" then
        req:OnAfterReload()        
    end
end

--[[@ [RequireModule/_] `require.reloadAll()`
    Reloads code of all loaded modules. All objects will use reloaded code.
@]]
function require.reloadAll()
    local loaded = {}
    for k,_ in pairs(require.cache) do
        if k:sub(1,1) == "/" then
            table.insert(loaded, k)
        end
    end
    for _,v in ipairs(loaded) do
        require.reload(v)        
    end
end

setmetatable(require, require)
