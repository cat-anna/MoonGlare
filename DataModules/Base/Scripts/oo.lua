-- part of MoonGlare engine internal module
-- object-oriented support for lua

oo = { }

function oo.Inherit(baseClass)
    local class = { }

    local isfName = type(baseClass) == "string"
    local base = baseClass
    if isfName then
        base = require(baseClass)
        class.BaseClassName = baseClass
    end

    class.IsClass = true
    class.BaseClass = base
    
    class.__index = class

    setmetatable(class, base)

    function class.New(...)
        local o = setmetatable({}, class)
        if o.Init then
            o:Init(...)
        end
        return o
    end

    return class
end

function oo.Class()
    return oo.Inherit()
end
