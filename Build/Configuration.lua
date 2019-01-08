--MoonGlare build subscript
--Engine configuration

function SetPCH(pch)
	filter { }

	pchheader(pch.hdr)
	pchsource(pch.src)

	filter "action:gmake"
		defines { "PCH_HEADER='\"" .. pch.hdr .. "\"'", }
	filter "action:vs*"
		defines { "PCH_HEADER=\"" .. pch.hdr .. "\"", }

	filter { }
end

function MoonGlare.SetOutputDir(subpath)
	filter "configurations:Debug"
		targetdir(dir.target .. "/" .. subpath)		
	filter "configurations:Release"
		targetdir(dir.target .. "/" .. subpath)		
	filter {}
end

local function SetCommonConfig()
	editorintegration "on"
	configurations { "Debug", "Release", }
	platforms { "x32", "x64", }
	language "C++"

	links {
	}
	defines {
		"STARVFS_USE_ORBITLOGGER",
	--	"BOOST_NO_AUTO_PTR",
	}
	includedirs {
        dir.bin,
		dir.base,
		dir.base .. "Libs",
		dir.src,
		dir.src .. "Common",
	}

	basedir "."
	debugdir "."
	location(dir.project)

	floatingpoint "Fast"
	symbols "On"
	flags { "NoMinimalRebuild", "MultiProcessorCompile", }

	filter "system:windows"
		links { "opengl32", "glu32", "gdi32", }
		defines{ "WINDOWS", "OS_NAME=\"Windows\"", "_SCL_SECURE_NO_WARNINGS" }
		systemversion "latest"
	filter "system:linux"
		links { "GL", "GLU", "X11", "Xxf86vm", "Xrandr", "Xi", "rt", }
		defines{ "LINUX", "OS_NAME=\"Linux\"" }

	filter "action:gmake"
		links { }
		buildoptions "-std=c++0y"
		defines{
			"GCC",
		}
	filter "action:vs*"
		links { }
		defines{
			"MSVC",
            "_ENABLE_EXTENDED_ALIGNED_STORAGE",
		}
		disablewarnings {
			"4100", -- unreferenced formal parameter
			"4201", -- nameless struct/union
			--"4003", -- not enough actual parameters for macro TODO:REMOVE THIS
			"4127", -- conditional expression is constant
			"4200", -- nonstandard extension used: zero-sized array in struct/union
			--"4307", -- integral constant overflow
            --"4290", -- C++ exception specification ignored
		}
		buildoptions  { "/std:c++latest", }
		entrypoint "mainCRTStartup"

	filter "platforms:x32"
		architecture "x32"
		defines {
			"X32",
			"PLATFORM_NAME=\"x32\"",
		}
		libdirs { }		
	filter "platforms:x64"
		architecture "x64"
		defines {
			"X64",
			"PLATFORM_NAME=\"x64\"",
		}
		libdirs { }
		
	filter "configurations:Debug"
		targetdir(dir.target)
		defines {
			"DEBUG",
			"SPACERTTI_TRACK_INSTANCES",
			"CONFIGURATION_NAME=\"Debug\"",
		}
		flags { }
		optimize "Debug"
		warnings "Extra"
		runtime "Debug"

	filter "configurations:Release"
		targetdir(dir.target)		
		defines {
			"RELEASE",
			"CONFIGURATION_NAME=\"Release\"",
		}
		flags { }
		vectorextensions "SSE2"
		optimize "Speed"
		warnings "Default"
		runtime "Release"

	filter 'files:**.lua'
		buildmessage 'Processing %{file.name} with bin2c'
		buildcommands '%{bin2c} -o "%{cfg.objdir}%{file.basename}".lua.h -n %{file.basename}_lua "%{file.relpath}"'
		buildoutputs '%{cfg.objdir}/%{file.basename}.lua.h'

	filter { }
end

function MoonGlare.GenerateSolution(Name)
	local sol = workspace(Name)
	SetCommonConfig()
	return sol
end
