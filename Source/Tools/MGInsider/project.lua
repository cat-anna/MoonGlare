
if _ACTION == "vs2013" then

require "premake-qt/qt"
local qt = premake.extensions.qt

group "Tools"
	project "MGInsider"
	
		qt.enable()
		qtpath("d:/Programowanie/Qt/5.5/msvc2013/")
	--	qtheaderpath(settings.qt.headers)
		qtprefix "Qt5"
		qtmodules { "core", "gui", "widgets", "network" }
		qtlibsuffix ""	
		
		SetPCH { hdr = "pch.h", src = "pch.cpp", }	
	
		kind "WindowedApp" --"ConsoleApp"
		defines { 
			"_BUILDING_TOOL_", 
			"_BUILDING_MGINSIDER_", 
		}
		files { 
			"**", 
		}	
		includedirs {
			"."
		}
		links {
			"lua51jit",
            "OrbitLogger",
		}
	
end
