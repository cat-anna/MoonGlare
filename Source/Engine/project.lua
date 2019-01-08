
group ""

project "*"
	startproject "Engine"

project "Engine"
	kind "WindowedApp"
	MoonGlare.SetOutputDir("Engine")   

	SetPCH { hdr = "pch.h", src = "pch.cpp", }

	defines {
		"_BUILDING_ENGINE_",
		"BT_EULER_DEFAULT_ZYX",    
		"_FEATURE_EXTENDED_PERF_COUNTERS_",
	}

	files {
		"**",
		"../Config/**",
		"../Utils/**",
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
		"Foundation",
	--	"x2c",
	}

	postbuildcommands {
		[["%{cfg.targetdir}/../svfs" -m "%{cfg.targetdir}" -s "rm('.*%%.[^eEdD]..$')" -e RDCExporter:%{cfg.targetdir}/../Modules/Engine.rdc:/ ]],
	}
