static_lib "base"
	kind "StaticLib"
	files { "**" }

	-- SetPCH { hdr = "pch.h", src = "pch.cpp", }
	-- forceincludes {
		-- "pch.h",
	-- }

	defines {
		-- "BT_EULER_DEFAULT_ZYX",
		-- "_FEATURE_EXTENDED_PERF_COUNTERS_",
	}

	links {
		"orbit_logger"
	}

	uses { "lib_orbit_logger" }

    -- excludes {	
    --     "./**Test.*"
    -- }
	-- includedirs {
	-- 	"%{cfg.objdir}",		
	-- }
