
require "premake-qt/qt"
local qt = premake.extensions.qt

group "Tools"
project "MGEditor"

    qt.enable()
    filter "platforms:x32"
        qtpath(MoonGlare.GetBuildSetting({name = "qtPath", group="Qt"}))
    filter "platforms:x64"
        qtpath(MoonGlare.GetBuildSetting({name = "qtPath_x64", group="Qt"}))

    filter {}
    qtprefix "Qt5"
    qtmodules { "core", "gui", "widgets", "network" }

    SetPCH { hdr = "Tools/MGEditor/pch.h", src = "pch.cpp", }

    kind "WindowedApp"
    defines {
        "_BUILDING_TOOL_",
        "_BUILDING_MGEDITOR_",
    }
    files {
        "**",
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

        "Foundation",
        -- "x2c",
    }

    filter "action:vs*"
        buildoptions  { "/bigobj", }
