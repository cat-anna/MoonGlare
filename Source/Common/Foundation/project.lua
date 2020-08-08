
group "Common"

project "Foundation"
	kind "StaticLib"

	-- SetPCH { hdr = "pch.h", src = "pch.cpp", }
	forceincludes {
		"pch.h",
	}

	defines {
		"BT_EULER_DEFAULT_ZYX",
		"_FEATURE_EXTENDED_PERF_COUNTERS_",
	}

	links {
	}

	files {
		"**",
	}
    excludes {
        "./**Test.*"
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
