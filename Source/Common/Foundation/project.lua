
group "Common"

project "Foundation"
	kind "StaticLib"

	filter 'files:**.lua'
		buildmessage 'Processing %{file.name} with bin2c'
		buildcommands '%{bin2c} -o "%{cfg.objdir}%{file.basename}".lua.h -n %{file.basename}_lua "%{file.relpath}"'
		buildoutputs '%{cfg.objdir}%{file.basename}.lua.h'

	SetPCH { hdr = "pch.h", src = "pch.cpp", }
	forceincludes {
		"pch.h",
	}

	defines {
		"BT_EULER_DEFAULT_ZYX",
		"_FEATURE_EXTENDED_PERF_COUNTERS_",
	}

	files {
		"**",
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
