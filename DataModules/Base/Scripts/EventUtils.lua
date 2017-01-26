
EventUtils = { }

local EventHandlers = { }

function EventUtils.Enable(script)
    EventUtils.EnableCollision(script)
end

function EventUtils.EnableCollision(script)
    if script.CollisionEnter then
        script.OnCollisionEnterEvent = EventHandlers.OnCollisionEnterEvent
    end
    if script.CollisionEnter then
        script.OnCollisionLeaveEvent = EventHandlers.OnCollisionLeaveEvent
    end
end

function EventHandlers:OnCollisionEnterEvent(data)
    print(tostring(data.Object))
    return self:CollisionEnter {
        name = "test Enter",
    }
end

function EventHandlers:OnCollisionLeaveEvent(data)
    print(tostring(data.Object))
    return self:CollisionLeave {
        name = "test Leave",
    }
end
