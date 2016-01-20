--Moonglare main build subscript 

--package.path = package.path .. ";./build/?.lua"

dir.bin = dir.root .. "bin_" .. (_ACTION or "none") .. "/"
dir.base = dir.root .. "MoonGlare/"
dir.src = dir.base .. "Source/"

MoonGlare = {
}

--require "VersionRule"
--require "Repositories"
include "Configuration.lua"
include "Projects.lua"
include "Modules.lua"

function MoonGlareSolution(Name)
	if _PREMAKE_VERSION then
		MoonGlare.GenerateSolution(Name or "MoonGlareEngine")
		ModuleManager:init();
		MoonGlare.LookForProjects()
	else
		print "Premake5 not detected!"
		--MoonGlare.Initialize()
	end
end


