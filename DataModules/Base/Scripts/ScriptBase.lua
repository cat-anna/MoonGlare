
local class = oo.Class()

function class:OnEvent(event)
    local f = self[event.HandlerName]
    -- print("test", event.EventName, event.HandlerName, f or "NULL")
    if f then
        f(self, event)
    end
end

return class
