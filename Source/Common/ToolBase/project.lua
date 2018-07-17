require "premake-qt/qt"
local qt = premake.extensions.qt

group "Common"

project "ToolBase"
    kind "StaticLib"

    qt.enable()
    qtprefix "Qt5"
    qtmodules { "core", "gui", "widgets", "network" }

    filter "platforms:x32"
        qtpath(MoonGlare.GetBuildSetting({name = "qtPath", group="Qt"}))
    filter "platforms:x64"
        qtpath(MoonGlare.GetBuildSetting({name = "qtPath_x64", group="Qt"}))
    filter {}

    defines {
    }

    files {
        "**",
    }
    includedirs {
        ".",
    }

    filter "configurations:Debug"
        defines{
        }
    filter "configurations:Release"
        defines {
        }
