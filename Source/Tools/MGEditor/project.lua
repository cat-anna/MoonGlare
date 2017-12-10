
require "premake-qt/qt"
local qt = premake.extensions.qt

group "Tools"
    project "MGEditor"

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
            "../../**.x2c",

            "../Shared/Qt/**",
            "../Shared/Resources/*",
        }
        includedirs {
            ".",
            "../Shared/",
            "../Shared/Qt/",
            "../Shared/Resources/",
        }
        links {
            "OrbitLogger",
            "libSpace",
            "StarVFS",
            "svfslua",
        }

        filter "action:vs*"
            buildoptions  { "/bigobj", }
