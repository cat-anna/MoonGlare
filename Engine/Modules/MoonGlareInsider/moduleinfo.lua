-- Moonglare module build script

	define_module { 
		name = "MoonGlareInsider",
		type = "module",
		
		files = { "*.cpp", "*.h", },
		defines = { },
		libs = { 
			release = { },
			debug = { },
			common = { },
		},
		
		apigen = true,
		api = { },
	}
