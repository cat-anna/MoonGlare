
group ""

	project "*"
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
			"../Shared/x2c/**",
		}
		includedirs {
			".",
			"%{cfg.objdir}",
		}
		links {
			"libSpace",
			"OrbitLogger",
			"StarVFS",

            "Renderer",
            "Assets",

			"glfx",
		}

		Features.ApplyAll(Engine.Features)
--		prebuildcommands {
--			"verupdate --input=..\\..\\Source\\Version.xml --outputdir=..\\..\\Source\\Engine\\Core MoonGlareEngine",
--		}

		ModuleManager:AddModules("Engine")

		MoonGlare.X2CRule()
		--local fs = os.matchfiles("../Shared/x2c/**")
		--MoonGlare.X2C.Files(fs)
		--files {
		--	dir.bin .. "Engine/X2C.Process.lua",
		--}
		--local f = dir.bin .. "/Engine/X2C.Process.lua"
		--os.execute("echo  > " .. f)

		--for i,v in ipairs(fs) do
		--	local p = path.getabsolute(v)
		--	print(p)
		--	os.execute("echo " .. p .. " >> " .. f)
		--end

		filter "configurations:Debug"
			Features.ApplyAll(Engine.Debug.Features)
			defines(Engine.Debug.Defines)
		filter "configurations:Release"
			Features.ApplyAll(Engine.Release.Features)
			defines(Engine.Release.Defines)
