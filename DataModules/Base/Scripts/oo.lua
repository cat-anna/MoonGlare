-- part of MoonGlare engine internal module
-- object-oriented support for lua

oo = { }

function oo.Inherit(base)
    local class = { }

    class.__index = class
    class.BaseClass = base

    setmetatable(class, base)

    return class
end

function oo.Class()
    return oo.Inherit()
end
