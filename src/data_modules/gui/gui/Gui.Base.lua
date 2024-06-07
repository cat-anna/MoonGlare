local class = oo.Inherit("ScriptComponent")

class.Enabled = true

function class.CallIfFunction(Item, ...)
    if type(Item) == "function" then
        return Item(...)
    end
    return Item
end

return class
