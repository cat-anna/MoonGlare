

group ""
project "Test"
	kind "ConsoleApp"
	-- MoonGlare.SetOutputDir("Test")   

	defines {
	}

	files {
        "**",
		"../**Test.*",
	}
	includedirs {
		".",
		"%{cfg.objdir}",
	}
	links {
	}
