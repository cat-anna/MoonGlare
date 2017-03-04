--Moonglare build subscript
--Engine configuration

Features = {
	PerfCounters = {
		Define = "",
	},
	FolderContainer = {
		Define = "_IFS_FOLDER_CONTAINER_SUPPORT_",
	},
	TextureIntrospector = {
		Define = "_FEATURE_TEXTURE_INTROSCPECTOR_",
	},

	Apply = function(name)
		local f = Features[name]
		defines(f.Define);
	end,

	ApplyAll = function(list)
		table.foreach(list, function(k,v) Features.Apply(v); end)
	end,
}

Engine = {
	Features = {
		"PerfCounters",
	},

	Debug = {
		Defines = {
		},
		Features = {
		},
	},
	Release = {
		Defines = {
		},
		Features = {
		},
	},
}

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
	libdirs {
		"../../../../LibsC",
		"assimp/lib",
		"bullet3-master/lib/",
	}
	includedirs {
		"../../../../LibsC",
        dir.bin,
		dir.base,
		dir.src,
		dir.libsrc,
		dir.root .. "Libs",
		"Libs",
	}

	basedir "."
	debugdir "."
	targetdir(dir.bin)
	location(dir.bin)

	floatingpoint "Fast"
	flags { "NoMinimalRebuild", "MultiProcessorCompile", }

	-- rules { "VersionCompiler" }

	local GLOBAL_CONFIGURATION_FILE = dir.base .. "GlobalConfiguration.h"

	filter "system:windows"
		links { "opengl32", "glu32", "gdi32", }
		defines{ "WINDOWS", "OS_NAME=\"Windows\"" }
	filter "system:linux"
		links { "GL", "GLU", "X11", "Xxf86vm", "Xrandr", "Xi", "rt", }
		defines{ "LINUX", "OS_NAME=\"Linux\"" }

	filter "action:gmake"
		links { }
		buildoptions "-std=c++0y"
		defines{
			"GCC",
			"GLOBAL_CONFIGURATION_FILE='\"" .. GLOBAL_CONFIGURATION_FILE .. "\"'",
		}
	filter "action:vs*"
		links { }
		defines{
			"MSVC",
			"GLOBAL_CONFIGURATION_FILE=\"" .. GLOBAL_CONFIGURATION_FILE .. "\"",
		}
		disablewarnings {
			"4100", -- unreferenced formal parameter
			"4201", -- nameless struct/union
			"4003", -- not enough actual parameters for macro TODO:REMOVE THIS
			"4127", -- conditional expression is constant
			"4200", -- nonstandard extension used: zero-sized array in struct/union
			"4307", -- integral constant overflow
            "4290", -- C++ exception specification ignored
		}
		buildoptions  { "/std:c++latest", }

	filter "platforms:x32"
		architecture "x32"
		defines {
			"X32",
			"PLATFORM_NAME=\"x32\"",
		}
	filter "platforms:x64"
		architecture "x64"
		defines {
			"X64",
			"PLATFORM_NAME=\"x64\"",
		}

	filter "configurations:Debug"
		defines {
			"DEBUG",
			"SPACERTTI_TRACK_INSTANCES",
			"CONFIGURATION_NAME=\"Debug\"",
		}
		flags { "Symbols", }
		optimize "Debug"
		warnings "Extra"
		runtime "Debug"
		targetsuffix "d"

	filter "configurations:Release"
		defines {
			"RELEASE",
			"CONFIGURATION_NAME=\"Release\"",
		}
		flags { }
		vectorextensions "SSE2"
		optimize "Speed"
		warnings "Default"
		runtime "Release"

	filter { }
end

function MoonGlare.GenerateSolution(Name)
	local sol = workspace(Name)
	SetCommonConfig()
	return sol
end
