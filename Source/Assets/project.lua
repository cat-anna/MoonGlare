
group "EngineLibs"

	project "Assets"
		kind "StaticLib"

		SetPCH { hdr = "pch.h", src = "pch.cpp", }

        forceincludes {
            "pch.h",
        }

		defines {
			"_BUILDING_ENGINE_",
            "_BUILDING_ASSETS_",
			"BT_EULER_DEFAULT_ZYX",
			"_FEATURE_EXTENDED_PERF_COUNTERS_",
		}

		files {
			"**",
			"../Config/*.h",
            "../Shared/x2c/AssetSettings.x2c",
            "../Shared/x2c/Assets/*.x2c",
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
