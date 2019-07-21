
group ""      
    project "Renderer"
        kind "StaticLib"

        SetPCH { hdr = "pch.h", src = "pch.cpp", }

        forceincludes {
            "pch.h",
        }

        defines {
            "_BUILDING_ENGINE_",
            "BT_EULER_DEFAULT_ZYX",
            "_FEATURE_EXTENDED_PERF_COUNTERS_",
        }

        links {
            "Foundation",
        }

        files {
            "**",
            "../Config/*.h",
        }
        includedirs {
            ".",
            "%{cfg.objdir}",
            "libMath",
        }
