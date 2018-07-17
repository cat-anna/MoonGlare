require "premake-qt/qt"
local qt = premake.extensions.qt

group "Tools"
    project "recon"
        kind "WindowedApp"
        
        qt.enable()
        qtprefix "Qt5"
        qtmodules { "core", "gui", "widgets", "network" }

        filter "platforms:x32"
            qtpath(MoonGlare.GetBuildSetting({name = "qtPath", group="Qt"}))
        filter "platforms:x64"
            qtpath(MoonGlare.GetBuildSetting({name = "qtPath_x64", group="Qt"}))
        filter {}
    
        links {
            "ToolBase",
            "Foundation",
        }

        SetPCH { hdr = "Tools/recon/pch.h", src = "pch.cpp", }	
        
        defines { 
            "_BUILDING_TOOL_", 
            "_BUILDING_RECON_", 
        }
        includedirs {
            "../Shared/Qt/",
        }
        files { 
            "*",
            "../Shared/**.rc",
        }	
    