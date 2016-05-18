--part of MoonGlare engine internal module

ScriptComponent = { }
ScriptComponent.EntityTable = { }
ScriptComponent.FixedProcess = { }


function ScriptComponent.Step(data)

	ScriptComponent:RunFixedProcess(data)
end

function ScriptComponent:RunFixedProcess(data)
	local count = #self.FixedProcess
	local i = 1
	
	while i <= count do
		local obj = self.FixedProcess[idx]
		obj:Process(data)
		i = i + 1
	end
	
end

function ScriptComponent:SetFixedProcess(sender)
	local idx = #self.FixedProcess + 1
	local info = self.EntityTable[sender.Entity:GetIndex()]
	info.FixedProcess = idx
	self.FixedProcess[idx] = sender
end

function ScriptComponent.AllocScript(Entity, ClassName)
	local script = { }
	ScriptComponent.EntityTable[Entity:GetIndex()] = {
		script = script,
	}
	
	script.Entity = Entity
	
	script.Class = Scripts.GetClass(ClassName)
	--script.BaseClass = nil
	
	script.Instance = {
		Entity = Entity,
	}
	
	local initf = script.Class.Init
	if initf then
		initf(script.Instance)
	end
	
	return true
end
