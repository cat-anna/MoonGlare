-- Moonglare module build script

	define_module { 
		name = "BassSound",
		type = "module",
		
		files = { "*.cpp", "*.h", },
		defines = { },
		libs = { 
			release = { },
			debug = { },
			common = { "bass", },
		},
		
		apigen = true,
		api = { },
	}
