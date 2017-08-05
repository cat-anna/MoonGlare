-- part of MoonGlare engine internal module
-- object-oriented support for lua

oo = { }

function oo.Inherit(base)
    local class = { }

    class.__index = class
    class.BaseClass = base

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
