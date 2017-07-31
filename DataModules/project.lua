
group "Modules"

    project "Debug"
        kind "Makefile"
        files "Debug/**"
        dependson "svfs"

        buildcommands {
            [["%{cfg.targetdir}/svfs" -m "../../../../MoonGlare/DataModules/Debug" -e RDCExporter:%{cfg.targetdir}/Debug.rdc:/]],
        }

    project "Base"
        kind "Makefile"
        files "Base/**"
        dependson "svfs"

        buildcommands {
            [["%{cfg.targetdir}/svfs" -m "../../../../MoonGlare/DataModules/Base" -e RDCExporter:%{cfg.targetdir}/Base.rdc:/]],
        }
