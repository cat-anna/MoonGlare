-- Moonglare module build script

	define_module { 
		name = "TrueTypeFont",
		type = "module",
		
		files = { "*.cpp", "*.h", },
		defines = { },
		libs = { 
			release = { "freetype255MT" },
			debug = {  "freetype255MTd" },
			common = { },
		},
		
		apigen = true,
		api = { },
	}
