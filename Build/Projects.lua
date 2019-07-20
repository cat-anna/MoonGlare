
function MoonGlare.LookForProjects()
	local i,v
	for i,v in ipairs(os.matchfiles(dir.root .. "Doc/project.lua")) do
		print("Found project: " .. path.getrelative(dir.root, v))
		dofile(v)
	end
	for i,v in ipairs(os.matchfiles(dir.root .. "Source/**/project.lua")) do
		print("Found project: " .. path.getrelative(dir.root, v))
		dofile(v)
	end
	for i,v in ipairs(os.matchfiles(dir.root .. "DataModules/project.lua")) do
		print("Found project: " .. path.getrelative(dir.root, v))
		dofile(v)
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

	local projbin = dir.project .. "%{prj.group}/%{prj.name}"
	location (projbin)
	includedirs {
		projbin,
	}
	defines {
		"_FEATURE_EXTENDED_PERF_COUNTERS_",
	}
    debugdir(dir.bin)

	return proj
end

function StaticLib(name)
	group "lib"
	local proj = project(name)
	kind "StaticLib"

	return proj
end