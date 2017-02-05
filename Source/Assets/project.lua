
group "EngineLibs"

	project "Assets"
		kind "StaticLib"

		SetPCH { hdr = "pch.h", src = "pch.cpp", }

		defines {
			"_BUILDING_ENGINE_",
            "_BUILDING_ASSETS_",
			"BT_EULER_DEFAULT_ZYX",
			"_FEATURE_EXTENDED_PERF_COUNTERS_",
		}

        MoonGlare.X2CRule()

		files {
			"**",
			"../Config/*.h",
			"../Utils/**.h",
            "../Shared/x2c/AssetConfiguration.x2c",
		}
		includedirs {
			".",
			"%{cfg.objdir}",
		}

		filter "configurations:Debug"
			defines{
            }
		filter "configurations:Release"
			defines {
            }
