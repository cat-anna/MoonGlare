--Moonglare main build subscript 

--package.path = package.path .. ";./build/?.lua"

dir.bin = dir.root .. "bin_" .. (_ACTION or "none") .. "/"
dir.base = dir.root .. "MoonGlare/"
dir.src = dir.base .. "Source/"
dir.libsrc = dir.base .. "Libs/"

MoonGlare = {
}

include "CustomRules.lua"
--require "Repositories"
include "Configuration.lua"
include "Projects.lua"
include "Libraries.lua"
include "Modules.lua"
include "Settings.lua"

function MoonGlareSolution(Name)
	if _PREMAKE_VERSION then
		MoonGlare.LoadSettings()
		MoonGlare.GenerateSolution(Name or "MoonGlareEngine")
		MoonGlare.CheckLibraries()
		ModuleManager:init()
		MoonGlare.LookForProjects()
		MoonGlare.SaveSettings()
	else
		print "Premake5 not detected!"
		--MoonGlare.Initialize()
	end
end


