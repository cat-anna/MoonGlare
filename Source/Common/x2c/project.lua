
local x2coutput = dir.project .. "Foundation/x2c/obj/%{cfg.platform}/%{cfg.buildcfg}/"

workspace()
    includedirs { x2coutput, }

group "Common"

project "x2c"
    kind "StaticLib"
    -- MoonGlare.SetOutputDir("Engine")   

    defines { }

    -- files {  }
    includedirs { }
    links { }

    files {
        x2coutput .. "x2c.h",
        -- x2coutput .. "x2c.cpp",
    }

    local x2cfiles =  os.matchfiles("**.x2c")
    for _,v in pairs(x2cfiles) do
        local fn = path.getbasename(v)
        files { 
            v,
            x2coutput .. fn .. ".x2c.h",
            -- x2coutput .. fn .. ".x2c.cpp",
        }
    end

	filter 'files:**.x2c'
		buildmessage 'Processing %{file.name} with xml2cpp'
		buildcommands(bin.x2c .. ' --enable-all --input "%{file.relpath}" --output "%{cfg.objdir}%{file.basename}.x2c.h"')
        buildoutputs '%{cfg.objdir}%{file.basename}.x2c.h'

    filter "configurations:Debug"
    filter "configurations:Release"
