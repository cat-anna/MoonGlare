--Moonglare main build subscript 

--package.path = package.path .. ";./build/?.lua"

dir.bin = dir.root .. "bin/"
dir.project = dir.bin .. (_ACTION or "none") .. "/"
dir.target = dir.bin .. "/%{cfg.buildcfg}_%{cfg.architecture}"
dir.base = dir.root .. "./"
dir.src = dir.base .. "Source/"
dir.libsrc = dir.base .. "Libs/"

MoonGlare = {
}

--require "Repositories"
include "Configuration.lua"
include "Projects.lua"

include "Modules.lua"
include "Settings.lua"

if _PREMAKE_VERSION then
	MoonGlare.LoadSettings()
	MoonGlare.GenerateSolution(Name or "MoonGlareEngine")
	include(dir.libsrc .. "LibProjects.lua")   
	ModuleManager:init()
	MoonGlare.LookForProjects()
	MoonGlare.SaveSettings()
else
	print "Premake5 not detected!"
	--MoonGlare.Initialize()
end
