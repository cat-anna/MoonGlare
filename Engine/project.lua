
group ""
	startproject "Engine"
		
	project "Engine"
		kind "ConsoleApp"

		SetPCH { hdr = "pch.h", src = "../pch.cpp", }	

		defines { 
			"_BUILDING_ENGINE_", 
			"BT_EULER_DEFAULT_ZYX",
			"_FEATURE_EXTENDED_PERF_COUNTERS_",
			
			"_FEATURE_FOLDER_CONTAINER_SUPPORT_",
		}

		files {
			"**",
			"../*", 
			"../Config/**",
			"../Utils/**",
			"../Libs/**",
		}
		includedirs {
			"."
		}
		links {
			"lua51jit",
			"freeimage", 
			"glfw3dll", 
			"glew32",
			"glfx", 						
			"assimp",
		}

		Features.ApplyAll(Engine.Features)
--		prebuildcommands { 
--			"verupdate --input=..\\..\\Source\\Version.xml --outputdir=..\\..\\Source\\Engine\\Core MoonGlareEngine",
--		}

		ModuleManager:AddModules("Engine")	
		
		filter "configurations:Debug"
			Features.ApplyAll(Engine.Debug.Features)
			defines(Engine.Debug.Defines)
		filter "configurations:Release"
			Features.ApplyAll(Engine.Release.Features)
			defines(Engine.Release.Defines)