--part of MoonGlare engine internal module

--[[
Interfaces = { 
	Script = oo.NewClass(),
}

Scripts = {
	Table = { }
}

local Table = Scripts.Table
local ScriptInterface = Interfaces.Script

function Scripts.Register(data)
	if not data.Name then
		error("Attempt to register nameless script")
		return
	end
	
	if not data.Class then 
		error("Attempt to register classless script")
		return
	end
	
	local Script = Table[data.Name]
	if not Script then
		Script = { }
		Table[data.Name] = Script
	end
	
	if Script.Class then
		warning("Overwritting script " .. data.Name)
	end
	
	Script.Class = data.Class
	
	return true
end

function Scripts.GetClass(name)
	if not name then
		error("Attempt to get nameless script")
		return
	end
	
	local s = Table[name]
	if not s then
	error("Script ", name, " does not exists!")
	end
	return s
end

----------------------------------------------

function ScriptInterface:SetFixedProcess()
	return ScriptComponent:SetFixedProcess(self)
end

function ScriptInterface:SetPositionXYZ(x,y,z)
	
end

]]