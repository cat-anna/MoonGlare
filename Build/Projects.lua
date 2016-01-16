
function MoonGlare.LookForProjects()
	local i,v
	for i,v in ipairs(os.matchfiles(dir.base .. "/**/project.lua")) do
		print("Found project: " .. v)
		dofile(v)
	end
end

local PremakeGroup = group
local CurrentGroup = ""

function group(Name)
	if not Name then
		return CurrentGroup
	end
	CurrentGroup = Name;
	PremakeGroup(Name)
	return Name
end

local PremakeProject = project

function project(Name)
	if not Name then
		return PremakeProject()
	end

	local proj = PremakeProject(Name)
	
	local projbin = dir.bin .. CurrentGroup .. "/" .. Name
	location (projbin)
	includedirs(projbin)
	
	return proj
end