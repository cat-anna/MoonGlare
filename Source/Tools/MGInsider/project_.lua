
require "premake-qt/qt"
local qt = premake.extensions.qt

group "Tools"
	project "MGInsider"

		qt.enable()
		qtpath(MoonGlare.GetBuildSetting({name = "qtPath", group="Qt"}))
	--	qtheaderpath(settings.qt.headers)"d:/Programowanie/Qt/5.5/msvc2013/"
		qtprefix "Qt5"
		qtmodules { "core", "gui", "widgets", "network", }
	--	qtlibsuffix ""	

		SetPCH { hdr = "Tools/MGInsider/pch.h", src = "pch.cpp", }

		kind "WindowedApp" --"ConsoleApp"
		defines {
			"_BUILDING_TOOL_",
			"_BUILDING_MGINSIDER_",
		}
		files {
			"**",
			"../Shared/Qt/**",
			"../Shared/Resources/*",
			"../../Source/Shared/MoonGlareInsider/**",
		}
		includedirs {
			".",
			"../Shared/Qt/",
		}
		links {
			"lua51jit",
            "OrbitLogger",
			"libSpace",
		}
