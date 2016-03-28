--Moonglare main build subscript 

--package.path = package.path .. ";./build/?.lua"

dir.bin = dir.root .. "bin_" .. (_ACTION or "none") .. "/"
dir.base = dir.root .. "MoonGlare/"
dir.src = dir.base .. "Source/"
dir.libsrc = dir.base .. "Libs/"

MoonGlare = {
}

--require "VersionRule"
--require "Repositories"
include "Configuration.lua"
include "Projects.lua"
include "Libraries.lua"
include "Modules.lua"

function MoonGlareSolution(Name)
	if _PREMAKE_VERSION then
		MoonGlare.GenerateSolution(Name or "MoonGlareEngine")
		MoonGlare.CheckLibraries()
		ModuleManager:init()
		MoonGlare.LookForProjects()
	else
		print "Premake5 not detected!"
		--MoonGlare.Initialize()
	end
end


