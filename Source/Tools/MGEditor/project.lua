
require "premake-qt/qt"
local qt = premake.extensions.qt

group ""
project "MGEditor"

    enable("qt")

    -- SetPCH { hdr = "Tools/MGEditor/pch.h", src = "pch.cpp", }

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
        --"%{cfg.objdir}",
    }
    links {
        "OrbitLogger",
        "libSpace",
        "StarVFS",

        "Foundation",
        "ToolBase",
    }

    filter "action:vs*"
        buildoptions  { "/bigobj", }
