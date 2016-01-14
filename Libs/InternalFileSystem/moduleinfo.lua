-- Moonglare module build script

	define_module { 
		name = "InternalFileSystem",
		type = "lib",
		
		files = { "**.cpp", "**.h", },
		defines = { },
		libs = { 
			release = { },
			debug = { },
			common = { },
		},
		
		apigen = false,
		api = { },
	}
