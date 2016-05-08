--part of MoonGlare engine internal module

Scripts = {
	Table = { }
}

local Table = Scripts.Table

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
	
	else
		Script.Class = {}
	end
	
	for k,v in pairs(data.Class) do
		Script.Class[k] = v
	end
	
	return true
end
