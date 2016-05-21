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
		local obj = self.FixedProcess[i]
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
	print("Alloc attempt")
	local classinfo = Scripts.GetClass(ClassName)
	local object = classinfo.Class:new()
	
	ScriptComponent.EntityTable[Entity:GetIndex()] = {
		object = object,
	}
	
	object.Entity = Entity
	
	if object.OnInit then
		object:OnInit()
	end
	
	return true
end
