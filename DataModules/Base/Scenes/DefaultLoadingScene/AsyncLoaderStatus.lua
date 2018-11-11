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
	self.Text:Set {
        Text = string.format("remain %d out of %d", js.pendingJobs, js.jobCount),
	}
end

function class:Step(data)
end

return class

