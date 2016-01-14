--Moonglare build subscript
--Engine configuration

MoonGlare.Configurations = {
	{
		Type = "Debug",
		Name = "Debug",
		Config = function()
			flags { "Symbols", }
			optimize "Debug"
			warnings "Extra"
			runtime "Debug"
		end,
	},
	{
		Type = "Debug",
		Name = "Test",
		Config = function()
			flags { "Symbols", }
			optimize "On"
			warnings "Extra"
			runtime "Debug"	
		end,
	},
	{
		Type = "Release",
		Name = "Release",
		Config = function()
			flags { }		
			vectorextensions "SSE2"
			optimize "Speed"
			warnings "Default"	
			runtime "Release"
		end,
	},	
	{
		Type = "Release",
		Name = "Performance",
		Config = function()
			flags { }		
			vectorextensions "SSE2"
			optimize "Speed"
			warnings "Default"	
			runtime "Release"
		end,
	},		
}

MoonGlare.GetConfigurationList = function ()
	local r = {}
	local k
	local v
	for k,v in pairs(MoonGlare.Configurations) do
		r[#r+1] = v.Name
	end
	return r;
end

Common = {
	LibDir = {
		"../../libs",
		"../../../LibsC",
		"assimp/lib", 
		"bullet3-master/lib/",
	},
	IncludeDir = {
		"../../libs",
		"../../../LibsC",	
		"source", 
		"source/Engine/", 
		"$(OBJDIR)",
		"bullet3-master/src",
		"libs",
	},
}

Libs = {
	Common = {
		"lua51jit",
		"freeimage", 
		"glfw3dll", 
		"glew32",
		"glfx", 						
		"assimp",
	},
	msvc = { },
	gcc = { },
	
	Linux = { "GL", "GLU", "GLEW", "X11", "Xxf86vm", "Xrandr", "Xi", "rt", },
	Windows = { "opengl32", "glu32", "gdi32", },
}

Features = {
	PerfCounters = {
		Define = "_FEATURE_EXTENDED_PERF_COUNTERS_",
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
		"FolderContainer",
		"PerfCounters",
	},
	Source = {
		"source/*",
		"source/Engine/**", 
		"source/Config/**",
		"source/Utils/**",
		"libs/**",
	},
	Defines = {
		"BT_EULER_DEFAULT_ZYX",
	},
	
	Debug = {
		Defines = {
		},
		Features = {
			"TextureIntrospector",
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
	defines { "PCH_HEADER=\"" .. pch.hdr .. "\"", }
end

function SetCommonConfig() 
	configurations(MoonGlare.GetConfigurationList())
	platforms { "x32", "x64" }
	language "C++"
	
	links(Libs.Common)
	libdirs(Common.LibDir)
	includedirs(Common.IncludeDir)
	
	basedir "."
	debugdir "."
	targetdir "bin"
	
	floatingpoint "Fast"
	
	flags { "NoMinimalRebuild", "MultiProcessorCompile" }
	
	-- rules { "VersionCompiler" }
	
	filter "system:windows"
		links(Libs.Windows)
		defines{ "WINDOWS", "OS_NAME=\"Windows\"" }
	filter "system:linux"
		links(Libs.Linux)	
		defines{ "LINUX", "OS_NAME=\"Linux\"" }
		
	filter "action:gmake"
		links(Libs.gcc)
		buildoptions "-std=c++0x"
	filter "action:vs*"
		links(Libs.msvc)
		disablewarnings { 
			"4100", -- unreferenced formal parameter
			"4201", -- nameless struct/union
			}
		
	filter "platforms:x32" 
		architecture "x32"
		defines { "X32", "PLATFORM_NAME=X32" }
	filter "platforms:x64" 
		architecture "x64"		
		defines { "X64", "PLATFORM_NAME=X64" }		
	
	
	local CommonSettings = {
		Release = function()
			defines { "RELEASE", }
		end,
		Debug = function()
			defines { "DEBUG", "GABI_TRACK_INSTANCES", }
		end,
	}	
	
	local k
	local v
	for k,v in pairs(MoonGlare.Configurations) do
		filter("configurations:" .. v.Name)
		defines("CONFIGURATION_NAME=\"" .. v.Name .. "\"")
		CommonSettings[v.Type]()
		v.Config()
	end
--[[	
	filter "configurations:Debug"
		defines { "DEBUG", "GABI_TRACK_INSTANCES", "CONFIGURATION_NAME=\"Debug\"" }
		flags { "Symbols", }
		optimize "Debug"
		warnings "Extra"
		runtime "Debug"
	filter "configurations:DebugTest"
		defines { "DEBUG", "GABI_TRACK_INSTANCES", "CONFIGURATION_NAME=\"DebugTest\"" }
		flags { "Symbols", }
		optimize "On"
		warnings "Extra"
		runtime "Release"		
	filter "configurations:Release"
		defines { "RELEASE", "CONFIGURATION_NAME=\"Release\""}
		flags { }		
		vectorextensions "SSE2"
		optimize "Speed"
		warnings "Default"	
		runtime "Release"
	filter "configurations:ReleasePerformance"
		defines { "RELEASE", "CONFIGURATION_NAME=\"ReleasePerformance\""}
		flags { }		
		vectorextensions "SSE2"
		optimize "Speed"
		warnings "Default"	
		runtime "Release"
		
		]]
	filter { }
end

function MoonGlareSolution(Name)
	local sol = solution(Name)
	SetCommonConfig()
	return sol
end
