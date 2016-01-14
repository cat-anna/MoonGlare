-- Moonglare module build script

	define_module { 
		name = "GameApplication",
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
