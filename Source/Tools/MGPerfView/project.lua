
require "premake-qt/qt"
local qt = premake.extensions.qt

group "Tools"
project "MGPerfView"

    enable("qt")

   -- -- SetPCH { hdr = "Tools/MGPerfView/pch.h", src = "pch.cpp", }

    kind "WindowedApp"
    defines {
        "_BUILDING_TOOL_",
        "_BUILDING_MGEDITOR_",
    }
    files {
        "**",
        "../Shared/Resources/*",
    }
    includedirs {
        ".",
        "../Shared/",
        "../Shared/Resources/",
    }
    links {
        "OrbitLogger",
        "libSpace",

        "Foundation",
        "ToolBase",
    }

    filter "action:vs*"
        buildoptions  { "/bigobj", }
