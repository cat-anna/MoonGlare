--MoonGlare Engine premake5 build script generator

include "build/MoonGlare.lua"

MoonGlareSolution "moonglare"
	startproject "moonglare"
		
	project "moonglare"
		kind "ConsoleApp"
		location "bin/moonglare"
		SetPCH { hdr = "pch.h", src = "source/pch.cpp", }	
		defines { "_BUILDING_ENGINE_", }
		defines ( Engine.Defines )
		files(Engine.Source)
		Features.ApplyAll(Engine.Features)
		prebuildcommands { 
			"verupdate --input=..\\..\\Source\\Version.xml --outputdir=..\\..\\Source\\Engine\\Core MoonGlareEngine",
		}
		ModuleManager:addLibs()
		ModuleManager:addSources()			
		filter "configurations:Debug"
			Features.ApplyAll(Engine.Debug.Features)
			defines(Engine.Debug.Defines)
		filter "configurations:Release"
			Features.ApplyAll(Engine.Release.Features)
			defines(Engine.Release.Defines)
			
			
	