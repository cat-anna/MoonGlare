require "premake-qt/qt"
local qt = premake.extensions.qt

group "Common"

project "ToolBase"
    kind "StaticLib"

    enable("qt")




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
