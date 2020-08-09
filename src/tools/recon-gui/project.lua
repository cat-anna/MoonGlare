require "premake-qt/qt"
local qt = premake.extensions.qt

group "Tools"
    project "MGReconGui"
        kind "WindowedApp"
        
        enable("qt")

        links {
            "OrbitLogger",
            "libSpace",

            "ToolBase",
            "Foundation",
        }

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
    