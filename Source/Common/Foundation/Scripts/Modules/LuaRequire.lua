
local org_require = require
     
require = {  }
require.__index = require
require.__call = function(self, ...) return org_require(...) end
require.cache = require_cache
require_cache = nil
                                                            
function require.reload(name)
    local req = require(name)
    local new = dofile(name)

    if type(req.onBeforeReload) == "function" then
        req:onBeforeReload()        
    end

    for k,v in pairs(new) do
        req[k] = v
    end

    if type(req.onAfterReload) == "function" then
        req:onAfterReload()        
    end
end

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
