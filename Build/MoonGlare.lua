--Moonglare main build subscript 


package.path = package.path .. ";./build/?.lua"

MoonGlare = {
}

require "VersionRule"
require "Repositories"
require "Configuration"
require "Modules"

if _PREMAKE_VERSION then
	ModuleManager:init();
else
	MoonGlare.Initialize()
end

