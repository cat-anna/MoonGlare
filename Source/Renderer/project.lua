
group "EngineLibs"

	project "Renderer"
		kind "StaticLib"

		SetPCH { hdr = "pch.h", src = "pch.cpp", }

        forceincludes {
            "pch.h",
        }

		defines {
			"_BUILDING_ENGINE_",
			"BT_EULER_DEFAULT_ZYX",
			"_FEATURE_EXTENDED_PERF_COUNTERS_",
		}

		files {
			"**",
			"../Config/*.h",
			"../Utils/**.h",
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
