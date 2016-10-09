
function MoonGlare.LookForProjects()
	local i,v
	for i,v in ipairs(os.matchfiles(dir.root .. "/**/project.lua")) do
		local reltative = path.getrelative(dir.root, v)
		local find = reltative:find("MoonGlare/Libs/")
		if find then
			print("Skiping project: " .. reltative)
		else
			print("Found project: " .. reltative)
			dofile(v)
		end
	end
end

local PremakeGroup = group
local CurrentGroup = ""

function group(Name)
	if not Name then
		CurrentGroup = ""
		return PremakeGroup()
	end
	CurrentGroup = Name .. "/";
	PremakeGroup(Name)
	return Name
end

local PremakeProject = project

function project(Name)
	if not Name then
		return PremakeProject()
	end

	if Name == "*" then
		return PremakeProject("*")
	end

	local proj = PremakeProject(Name)

	local projbin = dir.bin .. CurrentGroup .. Name
	location (projbin)
	includedirs(projbin)

	return proj
end
