

local premake_group_function = group
local CurrentGroup = ""

function group(name)
	if not name then
		CurrentGroup = ""
		return premake_group_function()
	end
	CurrentGroup = name .. "/";
	premake_group_function(name)
	return name
end

local premake_project_function = project

function project(name)
	if not name or name == "*" then
		return premake_project_function(name)
	end

	local proj = premake_project_function(name)
	local projbin = MoonGlare.path.project .. "/%{prj.group}/%{prj.name}"

	location(projbin)
	debugdir(MoonGlare.path.bin)

	-- local target_dir = string.format("%s/%s/%s", MoonGlare.path.target, CurrentGroup, name)
	-- filter "configurations:Debug"
	-- 	targetdir(Mtarget_dir)		
	-- filter "configurations:Release"
	-- 	targetdir(target_dir)		
	-- filter {}
	
	includedirs {
		projbin,
	}
	defines { }

	return proj
end

function StaticLib(name)
	group "lib"
	local proj = project(name)
	kind "StaticLib"

	return proj
end

function tool_project(name)
	group "Tools"
	local proj = project(name)
	defines {
		"_BUILDING_TOOL_",
	}
	return proj	
end

local function load_all_projects(start_dir)
	local disable_projects = MoonGlare.configuration.disable_projects
	for i,v in ipairs(os.matchfiles(start_dir .. "/**/project.lua")) do
		local relative = path.getrelative(MoonGlare.path.root, v)
		if disable_projects[relative] then
			print("Found disabled project: " .. relative)
		else
			print("Found project: " .. relative)
			include(v)
		end
	end
end

load_all_projects(MoonGlare.path.root)

-- 	local i,v
-- 	for i,v in ipairs(os.matchfiles(dir.root .. "Doc/project.lua")) do
-- 		print("Found project: " .. path.getrelative(dir.root, v))
-- 		dofile(v)
-- 	end
-- 	for i,v in ipairs(os.matchfiles(dir.root .. "Source/**/project.lua")) do
-- 		print("Found project: " .. path.getrelative(dir.root, v))
-- 		dofile(v)
-- 	end
-- 	for i,v in ipairs(os.matchfiles(dir.root .. "DataModules/project.lua")) do
-- 		print("Found project: " .. path.getrelative(dir.root, v))
-- 		dofile(v)
-- 	end	
-- end