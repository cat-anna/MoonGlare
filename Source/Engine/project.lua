
group ""

project "*"
    startproject "Engine"

project "Engine"
    kind "WindowedApp"
    MoonGlare.SetOutputDir("Engine")   

    SetPCH { hdr = "pch.h", src = "pch.cpp", }

    dependson "x2c"

    defines {
        "_BUILDING_ENGINE_",
        "BT_EULER_DEFAULT_ZYX",    
        "_FEATURE_EXTENDED_PERF_COUNTERS_",
    }

    files {
        "**",
        "../Config/**",
    }
    includedirs {
        ".",
        "%{cfg.objdir}",
    }
    links {
        "libSpace",
        "OrbitLogger",
        "StarVFS",

        "Renderer",
        "Foundation",
    --	"x2c",
    }

group "Modules"
    project "EngineBinaries"
        kind "Utility"
        MoonGlare.SetOutputDir("Modules")
        dependson{ "svfs", "Engine", }

        postbuildcommands {
            [["%{cfg.targetdir}/../svfs" -m "%{cfg.targetdir}/../Engine/" -s "rm('.*%%.[^eEdD]..$')" -e RDCExporter:%{cfg.targetdir}/Engine.rdc:/ ]],
        }
