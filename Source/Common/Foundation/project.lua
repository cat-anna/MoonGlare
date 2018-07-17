
group "Common"

project "Foundation"
	kind "StaticLib"

	-- SetPCH { hdr = "pch.h", src = "pch.cpp", }
	-- forceincludes {
		-- "pch.h",
	-- }

	defines {
		"BT_EULER_DEFAULT_ZYX",
		"_FEATURE_EXTENDED_PERF_COUNTERS_",
	}

	files {
		"**",
	}
	includedirs {
		".",
	}

	filter "configurations:Debug"
		defines{
		}
	filter "configurations:Release"
		defines {
		}
