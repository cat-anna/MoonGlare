
group ""

project "*"
	startproject "Engine"

project "Engine"
	kind "WindowedApp"
	MoonGlare.SetOutputDir("Engine")   

	filter 'files:**.lua'
		buildmessage 'Processing %{file.name} with bin2c'
		buildcommands 'bin2c -o "%{cfg.objdir}%{file.basename}".lua.h -n %{file.basename}_lua "%{file.relpath}"'
		buildoutputs '%{cfg.objdir}%{file.basename}.lua.h'

	SetPCH { hdr = "pch.h", src = "../pch.cpp", }

	defines {
		"_BUILDING_ENGINE_",
		"BT_EULER_DEFAULT_ZYX",
		"_FEATURE_EXTENDED_PERF_COUNTERS_",
	}

	files {
		"**",
		"../*",
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
		"Assets",
		"Foundation",
		-- "x2c",
	}

	postbuildcommands {
		[["%{cfg.targetdir}/../svfs" -m "%{cfg.targetdir}" -s "rm('.*%%.[^eEdD]..$')" -e RDCExporter:%{cfg.targetdir}/../Modules/Engine.rdc:/ ]],
	}
