--part of MoonGlare engine internal module

Internal = { }
Internal.EntityTable = { }
Internal.Scripts = { }

function Internal.RunFixedProcess()
	
	local count = #Internal.Scripts
	local i = 1
	
	while i <= count do
	
		i = i + 1
	end
end

function Internal.AllocScript(Entity, ClassName)
	local script = { }
	local idx = #Internal.Scripts + 1;
	Internal.Scripts[idx] = script
	
	script.Entity = Entity
	
	Internal.EntityTable[Entity:GetIndex()] = idx
	
	script.Class = Scripts.GetClass(ClassName)
	script.Instance = {
		Entity = Entity,
	}
	if script.Class.Init then
		script.Class.Init(script.Instance)
	end
	
	return Handle.MakeScriptHandle(idx)
end
