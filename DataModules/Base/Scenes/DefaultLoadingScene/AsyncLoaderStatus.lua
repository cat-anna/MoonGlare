local class = oo.Inherit("ScriptComponent")

function class:OnCreate()
    self.Text = self.GameObject:GetComponent(Component.Text)
	self:SetInterval(500)
    self:SetStep(false)
    
    self.Text:Set {
        Text = ""
	}
end

function class:OnTimer(tid)
    local ldr = require "AsyncLoader"
    local js = ldr:GetStatus()
    local done = (js.jobCount - js.pendingJobs) / js.jobCount 
	self.Text:Set {
        Text = string.format("%2.0f%%", done * 100),
	}
end

function class:Step(data)
end

return class
