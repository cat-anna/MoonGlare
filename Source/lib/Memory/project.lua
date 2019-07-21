
StaticLib "libMemory"
    defines {}
    excludes {}
    files { "**" }
    excludes { "./**Test.*" }
    includedirs {
        ".",
        "%{cfg.objdir}",		
    }
