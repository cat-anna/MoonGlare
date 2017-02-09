
require "premake-qt/qt"
local qt = premake.extensions.qt

group "Tools"
	project "MGEditor"

		MoonGlare.X2CRule()

		filter { }

		qt.enable()
		qtpath(MoonGlare.GetBuildSetting({name = "qtPath", group="Qt"}))
	--	qtheaderpath(settings.qt.headers)"d:/Programowanie/Qt/5.5/msvc2013/"
		qtprefix "Qt5"
		qtmodules { "core", "gui", "widgets", "network" }
	--	qtlibsuffix ""

		SetPCH { hdr = "Tools/MGEditor/pch.h", src = "pch.cpp", }

		kind "WindowedApp"
		defines {
			"_BUILDING_TOOL_",
			"_BUILDING_MGEDITOR_",
		}
		files {
			"**",
			"../Shared/Qt/**",
			"../Shared/x2c/**",
			"../../Shared/x2c/**",
			"../Shared/Resources/*",
		}
		includedirs {
			".",
			"../Shared/Qt/",
			"../Shared/Resources/",
		}
		links {
			"lua51jit",
            "OrbitLogger",
			"libSpace",
			"StarVFS",
		}

        filter "action:vs*"
            buildoptions  { "/bigobj", }


--filter 'files:scripts/**.lua'
--	buildmessage 'Compiling %{file.relpath} with bin2c'
--	buildcommands 'bin2c -o "%{cfg.objdir}/%{file.basename}.lua.h" -n %{file.basename}_lua "%{file.relpath}" '
--	buildoutputs '%{cfg.objdir}/%{file.basename}.lua.h'
