--MoonGlare main build subscript 

--package.path = package.path .. ";./build/?.lua"

dir.base = dir.root 

dir.bin = dir.root .. "bin/"
dir.project = dir.bin .. (_ACTION or "none") .. "/"
dir.target = dir.bin .. "/%{cfg.buildcfg}_%{cfg.architecture}"
dir.lib = dir.base .. "Libs/"
dir.src = dir.base .. "Source/"
dir.tools = dir.base .. "Tools/"

bin = {
	x2c = "lua " .. dir.tools .. "xml2cpp/src/xml2cpp.lua"
}

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
	include(dir.lib .. "LibProjects.lua")   
	ModuleManager:init()
	MoonGlare.LookForProjects()
	MoonGlare.SaveSettings()
else
	print "Premake5 not detected!"
	--MoonGlare.Initialize()
end
